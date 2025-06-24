#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "neko.h"
#pragma comment(lib,"neko.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _DEBUG
	#define trace(...)  printf(__VA_ARGS__)
#else
	#define trace(...)
#endif

#define DIR_MAXPATH    128
#define ENTRIES         32

static struct monitor {
	int count;
	unsigned int bimap[ENTRIES / 32];
	HANDLE handles[ENTRIES];
	unsigned char directory[ENTRIES][DIR_MAXPATH];
} monitor;

#define ENTRY_FOREACH(x)     for(int x = 0; x < monitor.count; x++)
#define ENTRY_DIR(i)         (monitor.directory[i])
#define ENTRY_HANDLE(i)      (monitor.handles[i])
#define ENTRY_MARK_VALUE(i)  (monitor.bimap[i / 32]  & (1 << i % 32))
#define ENTRY_MARK_SET(i)    (monitor.bimap[i / 32] |= (1 << i % 32))
#define ENTRY_DIR_COPY(d, s) do { \
	strncpy(ENTRY_DIR(d), ENTRY_DIR(s), DIR_MAXPATH - 1); \
	ENTRY_DIR(d)[DIR_MAXPATH - 1] = 0; \
} while(0)

#define ENTRY_SWEEP(i)       FindCloseChangeNotification(ENTRY_HANDLE(i))


static void inline monitor_marks_cleanup()
{
	for (int i = 0; i < (sizeof(monitor.bimap) / sizeof(int)); i++) {
		monitor.bimap[i] = 0;
	}
}

/*
 * Clear the elements in the array that are no longer valid
 *
 * e.g : [1, 0, 0, 2, 0, 0, 0, 3] => [1, 2, 3]
 */
static void monitor_marks_sweep()
{
	int i = 0;
	int acc = 0;
	int count = monitor.count;
	while (i < count) {
		int cur = i++;
		if (ENTRY_MARK_VALUE(cur)) {
			acc++;
			continue;
		}

		ENTRY_SWEEP(cur);

		while (i < count) {
			int next = i++;
			if (ENTRY_MARK_VALUE(next) == 0) {
				ENTRY_SWEEP(next);
				continue;
			}
			int slot = acc++;
			ENTRY_MARK_SET(slot);
			ENTRY_HANDLE(slot) = ENTRY_HANDLE(next);
			ENTRY_DIR_COPY(slot, next);
			break;
		}
	}
	monitor.count = acc;
}

static void monitor_init()
{
	monitor.count = 0;
}

static void monitor_release()
{
	ENTRY_FOREACH(i) {
		ENTRY_SWEEP(i);
	}
	monitor.count = 0;
}

static inline int isslash(int c)
{
	return c == '/' || c == '\\';
}
static void monitor_add_inner(unsigned char *dir)
{
	ENTRY_FOREACH(i) {
		if (strcmp(ENTRY_DIR(i), dir) == 0) { // already exists
			ENTRY_MARK_SET(i);
			return;
		}
	}
	trace("monitor_add_inner : '%s'\n", dir);
	HANDLE handle = FindFirstChangeNotificationA(dir, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
	if (handle == INVALID_HANDLE_VALUE) {
		/*
		 * ERROR_SUCCESS             : 0
		 * ERROR_INVALID_FUNCTION    : 1
		 * ERROR_FILE_NOT_FOUND      : 2
		 * ERROR_PATH_NOT_FOUND      : 3
		 * ERROR_TOO_MANY_OPEN_FILES : 4
		 * ERROR_ACCESS_DENIED       : 5
		 */
		fprintf(stderr, "monitor - HANDLE creation failed : 0x%x, for '%s'\n", GetLastError(), dir);
		return;
	}
	if (monitor.count >= ENTRIES) {
		FindCloseChangeNotification(handle);
		fprintf(stderr, "monitor - Out of bounds entries.");
		return;
	}
	int slot = monitor.count++;
	ENTRY_MARK_SET(slot);
	ENTRY_HANDLE(slot) = handle;
	strncpy(ENTRY_DIR(slot), dir, DIR_MAXPATH - 1);
	ENTRY_DIR(slot)[DIR_MAXPATH - 1] = 0;
}
static void monitor_add(char *path, int len)
{
	unsigned char dir[DIR_MAXPATH];
	dir[0] = 0;

	char *last = path + len - 1;
	if (path[0] == '.' && isslash(path[1])) // remove "./" at beginning
		path += 2;

	while (last >= path) {
		int c = *last;
		if (isslash(c)) {
			int cnt = (int)(last - path) % DIR_MAXPATH;
			strncpy(dir, path, cnt);
			dir[cnt] = 0;
			break;
		}
		last--;
	}
	if (dir[0] == 0) {
		dir[0] = '.';
		dir[1] = 0;
	}
	monitor_add_inner(dir);
}

/*
 * neko nml array : [inner, tostring, size]
 * inner : [
 *     [ entry_file_1; file_2; file_3; ...];
 *     [ entry_file_2; file_22; file_23; ...];
 *     [ entry_file_N; file_N2; file_N3; ...];
 * ]
 *
 * @param entries : neko nml array : Array<Array<String>>
 * @param entries : neko nml array : Array<Bool>
 * @return : bool
 */
#define VAL_ARRAY_SIZE(v)  val_int(       val_array_ptr(v)[2] )
#define VAL_ARRAY_PTR(v)   val_array_ptr( val_array_ptr(v)[0] )

static value watch(value entries, value filters)
{
	FILETIME ctime;
	GetSystemTimeAsFileTime(&ctime);

	monitor_marks_cleanup();

	for (int i = 0; i < VAL_ARRAY_SIZE(entries); i++) {
		value entry = VAL_ARRAY_PTR(entries)[i];

		if (val_is_null(entry))
			break;

		for (int j = 0; j < VAL_ARRAY_SIZE(entry); j++) {
			value nstr = VAL_ARRAY_PTR(entry)[j];
			int len = val_strlen(nstr);
			if (len == 0)
				continue;
			monitor_add(val_string(nstr), len);
		}
	}

	monitor_marks_sweep();

	if (!monitor.count)
		return val_false;

	DWORD status, slot;
waitfor:
	status = WaitForMultipleObjects(monitor.count, monitor.handles, FALSE, INFINITE);
	slot = status - WAIT_OBJECT_0;
	if (slot >= (DWORD)monitor.count) {
		fprintf(stderr, "monitor - WaitForMultipleObjects ERROR : 0x%x\n", status);
		monitor_release();
		return val_false;
	}
	FindNextChangeNotification(ENTRY_HANDLE(slot));
	// compares
	WIN32_FILE_ATTRIBUTE_DATA fattr;
	int found = 0;
	for (int i = 0; i < VAL_ARRAY_SIZE(entries); i++) {

		value entry = VAL_ARRAY_PTR(entries)[i];

		if (val_is_null(entry))
			break;

		VAL_ARRAY_PTR(filters)[i] = val_false;

		for (int j = 0; j < VAL_ARRAY_SIZE(entry); j++) {
			value nstr = VAL_ARRAY_PTR(entry)[j];
			int len = val_strlen(nstr);
			if (len == 0)
				continue;
			if (!GetFileAttributesExA(val_string(nstr), GetFileExInfoStandard, &fattr))
				continue;
			if (CompareFileTime(&fattr.ftLastWriteTime, &ctime) > 0) {
				found = 1;
				VAL_ARRAY_PTR(filters)[i] = val_true;
				if (j == 0) {
					printf("monitor - Changed : '%s'\n", val_string(nstr));
				} else {
					unsigned char *s = val_string(VAL_ARRAY_PTR(entry)[0]);
					printf("monitor - Changed : '%s' update '%s'\n", val_string(nstr), s);
				}
				break;
			}
		}
	}
	if (!found)
		goto waitfor;
	return val_true;
}

#ifdef _WINDLL

DEFINE_PRIM(watch, 2);

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD reason, LPVOID lpreserved)
{
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		monitor_init();
		break;
	case DLL_PROCESS_DETACH:
		monitor_release();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

#else

int main(int argc, char *argv[])
{
	monitor_init();
	value entries = alloc_array(3); // array<array<string>>, it's horrible
	{
		value inner = alloc_array(3);
		val_array_ptr(entries)[0] = inner;
		val_array_ptr(entries)[1] = val_null;
		val_array_ptr(entries)[2] = alloc_int(3);

		value e1_inner = alloc_array(1);
		val_array_ptr(e1_inner)[0] = alloc_string("hss/style.hss");
		value e1 = alloc_array(3);
		val_array_ptr(e1)[0] = e1_inner;
		val_array_ptr(e1)[1] = val_null;
		val_array_ptr(e1)[2] = alloc_int(val_array_size(e1_inner));

		value e2_inner = alloc_array(2);
		val_array_ptr(e2_inner)[0] = alloc_string("hss/dark.hss");
		val_array_ptr(e2_inner)[1] = alloc_string("hss/subs/_vars.hss");
		value e2 = alloc_array(3);
		val_array_ptr(e2)[0] = e2_inner;
		val_array_ptr(e2)[1] = val_null;
		val_array_ptr(e2)[2] = alloc_int(val_array_size(e2_inner));

		value e3_inner = alloc_array(3);
		val_array_ptr(e3_inner)[0] = alloc_string("./hss/light.hss");
		val_array_ptr(e3_inner)[1] = alloc_string("./hss/subs/_vars.hss");
		val_array_ptr(e3_inner)[2] = alloc_string("./hss/subs/colors.hss");
		value e3 = alloc_array(3);
		val_array_ptr(e3)[0] = e3_inner;
		val_array_ptr(e3)[1] = val_null;
		val_array_ptr(e3)[2] = alloc_int(val_array_size(e3_inner));

		val_array_ptr(inner)[0] = e1;
		val_array_ptr(inner)[1] = e2;
		val_array_ptr(inner)[2] = e3;
	}
	value filters = alloc_array(3);
	{
		val_array_ptr(filters)[0] = alloc_array(3);
		val_array_ptr(filters)[1] = val_null;
		val_array_ptr(filters)[2] = alloc_int(3);
	}
	while (1) {
		value updates = watch(entries, filters);
	}
	monitor_release();
	return 0;
}

#endif
