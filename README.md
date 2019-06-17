#### HSS is tool that extends the CSS syntax with powerful features such as variables and nested blocks.

## Usage

First you have to build it by `make` or `build.bat`(*if Windows*), it's depends on [Neko](http://nekovm.org/) or [Haxe](https://haxe.org/)

> Or Download HSS and put it in some place where it can be run from the commandline :
>* Windows version : <http://ncannasse.fr/file/hss-1.3-win.zip>
>* OSX version : <http://ncannasse.fr/file/hss-1.3-osx.gz>
>* Linux version : <http://ncannasse.fr/file/hss-1.3-linux.gz>
>
> **TIP:** On OSX and Linux you will have to enable the file to be executable after unzipping, by running the command chmod +x hss

You can then rename you _.css_ file with the extension _.hss_. Since HSS extends the CSS syntax, it means that every valid CSS valid is a valid HSS file as well.

You can then compile your HSS file into the corresponding CSS file by running the hss command :

```
hss myfile.hss
```
You can specify an output directory with `-output <dir>` if you need it to be different than the hss file one.

> **TIP:** Since every time the HSS file is modified the CSS file needs to be regenerated, it's better if the website itself automatically runs HSS when it notices that the HSS file has changed, and displays errors if any occurs.

## Syntax
HSS is a CSS compiler which supports valid CSS syntax, so for every error that occurs during the parsing of the HSS file, it will display and error indicating at which file and which line the error occurred.

HSS also enforce additional syntax rules. While this is valid CSS, the following is not HSS-valid since there is a missing semicolon at the end of the color value :
```
pre {
    color : #FF0000
}
```
In that case, you will get an error telling :

`myfile.hss:3: Unexpected '}' maybe missing ;`
Fix all the errors and HSS will then be able to generate the corresponding CSS file.

### Property Variables
HSS adds property variables to the CSS syntax.

You can declare a variable with the keyword var and use it in the HSS file by prefixing the variable name with a dollar :
```
var mycolor = #1111AA;
var myfont = "Trebuchet MS", Arial, sans-serif;

body {
    color : $mycolor;
    font-family : $myfont;
}
```
Variables are a very good way to avoid replacing everything in a CSS file every time you want to change a single color value. It also helps designers to remember and easily locate the main colors that are used in a given website.

### Block Variables
You can also declare variables that will declare several properties at the same time :
```
var nomargin = { margin : 0px; padding : 0px; }

pre {
    $nomargin;
    color : #FF0000;
}
```
Block variables are very useful to share some "behaviors" between unrelated rules. They can be used to improve the way CSS files are organized and makes it much more easy to test style-changes.

The Block Variables also has **function features**. Since 1.5.2. Note that it's uses **semicolon(`;`)** as the parameter separator.
because the comma(`,`) is a valid CSS property value.

```scss
var nomargin = { margin : $margin; padding : $padding;}
var pad-4 = 4px;
pre {
  $nomargin(margin = 16 + 2px; padding = -$pad-4); // (name = any css value; ...)
  color : #FF0000;
}
```

the output:

```css
pre {
  margin: 18px;
  padding: -4px;
  color: #FF0000;
}
```

Note that the "Block Variables" and "Property Variables" belong to different spaces.

```scss
var alpha = {            // Definition a "Block Variables".
  opacity: $alpha;       // Reference to "Property Variables", NOT "Block Variables"
  filter: alpha(opacity=$alpha*100);  // IE8 filter
  // $alpha;             // It's "Block Variables", but hss will report an error to avoid entering the dead loop.
}

.alpha-80 {
  $alpha(alpha = 0.8);   // use it.
}

var ref = $alpha;        // The right side of the expr will be parsed as "Property Var",
                         // So you can't refer to "Block Variables"
```

#### the variables scope

if the name of the "Block Variables" starts with `_` then the internal variables will be exported to the current scope.

```scss
var vars =  { var color = black;}
.without {
  var color = red;
  $vars();
  color: $color;
}

var _vars = { var color = black;}  // starts with "_"
.with {
  var color = red;
  $_vars();
  color: $color;
}
```

output:

```css
.without {
  color: red;
}
.with {
  color: black;
}
```

### Nested Blocks
One of the things that are the most annoying with CSS is when you want to avoid class-names clashing. For instance if you declare the following :
```
.name { font-weight : bold; }
```
Then there is an high possibility that class="name" is used in several different parts of the website, and needs to be styled differently depending on the case.

In order to avoid these problems, it's often needed to specify the context in which is class occurs, which often to the following declarations :
```css
.faq {
    color : #BC683C;
}

.faq .form {
    width : 100px;
}

.faq .form textarea {
    width : 100%;
    height : 80px;
}

.faq .form .name {
    font-weight : bold;
}

.faq ul {
    margin-left : 20px;
    margin-bottom : 20px;
}
```
In that CSS example, we can see how the natural nested XHTML structure has been "flattened", and how we need to specify before each block the complete context in which the class occurs.

HSS allows then a much more natural way of declaring the same thing, which is called nested blocks. Here's the previous example rewritten by using nested blocks :
```scss
.faq {
    color : #BC683C;
    .form {
        width : 100px;
        textarea {
            width : 100%;
            height : 80px;
        }
        .name {
            font-weight : bold;
        }
    }
    ul {
        margin-left : 20px;
        margin-bottom : 20px;
    }
}
```
As you can see, such nested structure provides much more readability and maintainability than flatten one. Using nested blocks is very good way to express complex structures and avoid class name clashing.

more complex:

```scss
a {
    var color = #0366d6;
    var margin = 4px;

    color: #666;
    &.disabled, &:disabled {
        color: lighten($color, 20%);   // color function: darken/lighten/saturate/desaturate/invert
    }
    &:hover > li {
        float: left;
    }
    > span, ~ i {
        font-size: 80%;
    }
    + a {
        margin-left: -$margin;
    }
    &:hover::after {
        content: " \f2b9 ";
    }
    &[href] {
        color: $color;
    }
    &:not([href]):nth-child(2n+1) {
        text-decoration: none;
    }
}
```

generated css:

```css
a {
    color: #666;
}
a.disabled, a:disabled {
    color: #439AFC;
}
a:hover > li {
    float: left;
}
a > span, a ~ i {
    font-size: 80%;
}
a + a {
    margin-left: -4px;
}
a:hover::after {
    content: " \f2b9 ";
}
a[href] {
    color: #0366d6;
}
a:not([href]):nth-child(2n+1) {
    text-decoration: none;
}
```

### Condition

Support a simple conditional statement that is to use `&&` and `||` to join exprs, and the left of the expr must be "Property Variable" or `-Dname[=value]`*(spaces are not allowed in the definetion)* from Console.

```scss
var polyfill = { background-color: blue; }
var ie8 = 1;                      // define property var or you can pass "-Die8" from Console
.one {
  $ie8 && $polyfill;              // ifdef $ie8 then $polyfill
}

var alpha = {
  opacity: ($alpha / 100);        // division only works with "()"
  filter: alpha(opacity = $alpha);
}
span {
  $flag || $alpha(alpha = 80);    // if not def $flag then $alpha
}
```
HSS will not detect the specific value of the left(Property) variable, only determine whether it is defined, and the right side can also be the following values:

```scss
$url && @import($url);   // @import, Note: @import only works in top level

.two {
  var blue = #00e;

  $blue && color: $blue; // A CSS Attribute: Value;

  $flags || {            // A Braces Block then all internal exprs will be moved to the current scope
    var blue = #00f;
    &:hover {
      color: $blue;
    }
  }

  $ie8 || $ie9 && {      // The left can be multiple conditions, but parentheses are not supported
    border: 0;
  }
}
```

the output is:

```css
.two {
  color: #00e;
}
.two:hover {
  color: #00f;
}
```

### Comments
There are two kind of comments possible in HSS, which are the same used in popular syntax such as C/Java. The `/* ... */` comment is a multiline comment which is already part of CSS and the `// ....` comment is used for single-line comment.
```scss
p {
//  color : red;
    font-weight : bold;
/*  margin : 0px;
    padding : 0px; */
}
```
All comments are removed from the CSS output.

### CSS Validation
Apart from validating the syntax and providing additional things, HSS also checks the validity of the CSS properties values. For instance the following will not be allowed because of the double 't' :
```
pre { font-weightt : bold; }
```
The following also will not be allowed by 'bald' is not a valid value for the property font-weight :
```
pre { font-weight : bald; }
```
HSS also enforces some good CSS practices such as :

 * properties declarations must always end with a semicolon (;)
 * URLs must be quoted : don't do url(img.gif) but please use url('img.gif') instead.

#### Notes

* HSS will try to detect if `@media` query is valid, but not all the syntax

  ```scss
  // you could use quotes to skip detection in media query.
  @media (some-css3-media: "unknown") {}

  // quotes for media type
  @media "tv", "handheld" {}

  // quotes almost everywhere
  @media "screen and (min-width: 30em) and (orientation: landscape)" {}
  @media screen and (min-width: 30em) "and (orientation: landscape)" {}
  @media screen "and (min-width: 30em)" and (orientation: landscape) {}

  // Note: Only supports Property Variables in value of the feature/attribute
  var narrow_width = 767px;
  @media only screen and (max-width : $narrow_width) {}
  ```

* CSS rule limit:
  - `font:` only accept single font-family value
  - `background:` no support for `/background-size` notation

### CSS Rules
The whole CSS properties rules that are used to check the property value correctness are listed in the Rules.nml file of the HSS sources. You might want to modify them for your own needs.

### Selectors
Here's the list of CSS selectors with their HSS support status, see here for the corresponding explanations :
```
E : supported
E F : supported
E > F : supported
E + F : supported
E ~ F : supported
DIV.warning : supported
E#myid : supported
E:first-child, E:link, E:hover and other CSS2 and 3 pseudo-classes selectors are supported
E[foo], E[foo=value], E[foo|=value], E[foo~=value], E[foo^=value], E[foo$=value], E[foo*=value] : supported
```
[supported pseudo-classes selectors](hss/Rules.nml#L46-L51), and other unsupported pseudo-selectors can use `CSS()` hack :
```css
/* Here CSS() only accepts string as argument */
.page CSS("h1:placeholder") {
    color : red
}
```

### Operations
It is possible to perform some operations between values :

* NOTE: **division**: (`/`) only works with `()`

```scss
var scale = 3;
.big {
    width : 50px + 20px;
    height : 30px * $scale;
    color : #FF0000 * 0.7;
    line-height: (120 / 100);
}
```
Operations between two different units (for instance 50px + 3em) are not allowed.

### Hacks Support

* `CSS(values)`: will simply output the values without rule checking which is sometimes useful for non-standard CSS properties:

  ```css
  .image {
    my-special-property : CSS("some specific value"); /* NOTE: The outer quotes will be strip if only single string */
    my-special-property : CSS(some specific value);   /* In most cases, quotes are not required, New in 1.6 */
    filter: CSS( "progid:DXImageTransform.Microsoft.DropShadow"(color=#88FF0000,offx=8,offy="8") ); /* Old IE filter */
  }
  ```

* `color : rgba(r,g,b,a); background-color : rgba(r,g,b,a)` : will add a solid color default value for browsers which don't support rgba

* `hss-width` and `hss-height` : will generate width and height from which will be subtracted the padding and border values declared in the current block.

* You can also add `@include('some css string')` either at the top level or instead of an attribute, this will include raw CSS string in the output, prefixed with the hierarchy classes if any New in 1.4

* `@import("rel_path/to/myhss")` can be used to import another hss file, or use `@import("rel_path/to/somelib.css")` to inject a CSS file directly. Duplicate imported files will be ignored.

  ```scss
  @import("path/to/_vars") // if file name starts with "_" the variables of the file will be exported to current file.
  @import("path/to/reset")
  ```

* hss functions:

  `darken, lighten, saturate, desaturate, invert`: for color adjustment similar to *scss/less*, but only accept `#RRGGBB` and `rgb(int,int,int)`

  ```scss
  body {
    var gray = #999;
    border-color: darken(rgb(255, 255, 255), 5%);
    color: lighten(invert($gray * 0.5), 5%);
  }
  ```

  `int(float)`: convert float to int, you will rarely use it.


* `embed("path/to/image")`: for embedding small(less than 24KB) image(png/jpg/gif) as `data:image/xxx;base64`.
  ```scss
  .logo {
    background-image: embed("logo.png"); // the png is relative to current .hss file.
  }
  ```

## Credits
The HSS software was developed by Nicolas Cannasse for Motion-Twin.
