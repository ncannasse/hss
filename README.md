#### HSS is tool that extends the CSS syntax with powerful features such as variables and nested blocks.

## Usage

1. Download and then build via `make` or `build.bat`

2. Copy both hss.exe(or hss) and [css.rules](bin/css.rules) to your application directory.

You can compile your HSS file into the corresponding CSS file by running the hss command :

```bash
#hss [options] <file>
# Options :
#  -output <dir> : specify output directory
#  -rule <file>  : specify a css rule set file
#  -D <flag>     : define a conditional compilation flag
#  --append      : available only if '-rule <file>' is specified
#  --minify      : minify output by removing some whitespaces
#  --stdout      : output to stdout
#  --sourcemap   : outputs a v3 sourcemap file
#
hss myfile.hss
```

Features:
* [Customizable CSS rules](#Customizable-CSS-rules) version 2.0+
* Variable
  - [Property Variables](#Property-Variables)
  - [Block Variables](#Block-Variables)
* [Nested Blocks](#Nested-Blocks)
* [Macro Condition](#Macro-Condition) *Only accept parameters passed via '-D flag'*
* [CSS Validation](#CSS-Validation)
* [Operations](#Operations)
* [More...](#Hacks-Support)

> **TIP:** Since every time the HSS file is modified the CSS file needs to be regenerated, it's better if the website itself automatically runs HSS when it notices that the HSS file has changed, and displays errors if any occurs.

## Syntax
HSS is a CSS compiler which supports valid CSS syntax, so for every error that occurs during the parsing of the HSS file, it will display and error indicating at which file and which line the error occurred.

HSS also enforce additional syntax rules. While this is valid CSS, the following is not HSS-valid since there is a missing semicolon at the end of the color value :
```css
pre {
    color : #FF0000
}
```
In that case, you will get an error telling :

`myfile.hss:3: Unexpected '}' maybe missing ;`
Fix all the errors and HSS will then be able to generate the corresponding CSS file.

### Customizable CSS rules

HSS 2.0 provides a new rule parser for loading external text rule files.

Usually, some game UI or Apps that use CSS have their own CSS syntax.

With this tool, you can customize some CSS rules to detect errors before loading.

And the new rules syntax is simpler than before.

```scss
// myui.rule
// global
{
    var unit   = Unit(dip px) // Unit() is used to define unit suffix
    var length = $unit float  // matches $unit or float
    var color  = #FFF #FFFFFF rgb(0-255, 0-255, 0-255);
    color  : $color;
    width  : $length;
    margin : $length{1, 4};
    filter : text-shadow($color, $length{2, 4});
}

// namespace
box {
    box-only : special;       // only available for <box>
}

// namespace list
box, button, image, :hover {
}
```

### Property Variables
HSS adds property variables to the CSS syntax.

You can declare a variable with the keyword var and use it in the HSS file by prefixing the variable name with a dollar :
```scss
var mycolor = #1111AA;
var myfont = "Trebuchet MS", Arial, sans-serif;

body {
    color : $mycolor;
    font-family : $myfont;
}

// scss syntax will also works
$mycolor : #ddd;
```
Variables are a very good way to avoid replacing everything in a CSS file every time you want to change a single color value. It also helps designers to remember and easily locate the main colors that are used in a given website.

### Block Variables
You can also declare variables that will declare several properties at the same time :
```scss
var nomargin = { margin : 0px; padding : 0px; }

pre {
    $nomargin;
    color : #FF0000;
}
```
Block variables are very useful to share some "behaviors" between unrelated rules. They can be used to improve the way CSS files are organized and makes it much more easy to test style-changes.

The Block Variables also has **function features**. Note that it's uses **semicolon(`;`)** as the parameter separator.

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

### Macro Condition

New in `1.7`, The condition value should be passed by using `-D flag` on the command line.

  > since `#` is a valid css query value, so here uses `@` as the macro prefix.

```less
div {
@if flag1
  color: #f00;
@elseif (firefox || !edge) // if "&&" or "||" then "()" is required.
  color: #0f0;
@else
  color: @if blue #00f @else #000 @end ;
@end
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

`@media` queries:

```scss
// you could use quotes to skip detection for "media type" or "media feature".
@media "tv", "handheld" {}
@media (some-css3-media: "unknown") {}
// Note: Only supports Property Variables in value of the "media feature"
var narrow_width = 767px;
@media only screen and (max-width : $narrow_width) {}
```

HSS does not support all CSS Attributes, such as `grid`series, So you may need `CSS()` to skip detection:

```scss
grid: CSS(auto-flow dense / 40px 40px 1fr);
```

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
.page CSS("h1:blank") { /* Here CSS() only accepts string as argument */
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

* `-D merge-media` : The `media queries` will be merged.

* The input name could be `stdin`.

* `CSS(values)`: will simply output the values without rule checking which is sometimes useful for non-standard CSS properties:

  ```css
  .image {
    my-special-property : CSS("some specific value"); /* NOTE: The outer quotes will be strip if only single string as argument */
    my-special-property : CSS(some specific value);   /* In most cases, quotes are not required */
    filter: CSS( "progid:DXImageTransform.Microsoft.DropShadow"(color=#88FF0000,offx=8,offy="8") ); /* Old IE filter */
  }
  ```

* `@import("rel_path/to/myhss")` can be used to import another hss file, or you can use `@import("rel_path/to/somelib.css")` to inject a CSS file directly. Duplicate files will be ignored.

  ```scss
  @import("path/to/_vars")

  @import "path/to/reset";
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

  `alphamix(foreground, alpha, background)`: Blend a translucent foreground color with a background color

  ```scss
  // reduce the use of "opacity" to improve browser performance
  menu {
    var background-color = #343a40;
    background-color: $background-color;
    button {
      var button-color = #007bff;
      var text-color   = #f8f9fa;
      var alpha = 0.4;

      color: alphamix($text-color, $alpha, $background-color);              // output: #82868A
      background-color: alphamix($button-color, $alpha, $background-color); // output: #1F548C

      &:hover {
        color: $text-color;
        background-color: $button-color;
      }
    }
  }
  ```

  `int(float)`: convert float to int

  `string(expr)`: convert expr to string

  ```scss
  var alpha = 0.3;
  // For IE8
  -ms-filter: string( "progid:DXImageTransform.Microsoft.Alpha"(Opacity=$alpha*100) );
  // [output] -ms-filter: "progid:DXImageTransform.Microsoft.Alpha(Opacity=30)"
  ```

* `embed("path/to/image")`: for embedding small(less than 24KB) image(png/jpg/gif) as `data:image/xxx;base64`.
  ```scss
  .logo {
    background-image: embed("logo.png"); // the png is relative to current .hss file.
  }
  ```

* `color : rgba(r,g,b,a); background-color : rgba(r,g,b,a)` : will add a solid color default value for browsers which don't support rgba

* `hss-width` and `hss-height` : will generate width and height from which will be subtracted the padding and border values declared in the current block.

* You can also add `@include('some css string')` either at the top level or instead of an attribute, this will include raw CSS string in the output, prefixed with the hierarchy classes if any New in 1.4

## Credits
The HSS software was developed by Nicolas Cannasse for Motion-Twin.
