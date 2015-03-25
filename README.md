#### HSS is tool that extends the CSS syntax with powerful features such as variables and nested blocks.

#Usage
First download HSS and put it in some place where it can be run from the commandline :

 * Windows version : <http://ncannasse.fr/file/hss-1.3-win.zip>
 * OSX version : <http://ncannasse.fr/file/hss-1.3-osx.gz>
 * Linux version : <http://ncannasse.fr/file/hss-1.3-linux.gz>

Sources are available in this repository, you will need [Neko](http://nekovm.org/) to compile them.

> **TIP:** On OSX and Linux you will have to enable the file to be executable after unzipping, by running the command chmod +x hss

You can then rename you _.css_ file with the extension _.hss_. Since HSS extends the CSS syntax, it means that every valid CSS valid is a valid HSS file as well.

You can then compile your HSS file into the corresponding CSS file by running the hss command :

```
hss myfile.hss
```
You can specify an output directory with `-output <dir>` if you need it to be different than the hss file one.

> **TIP:** Since every time the HSS file is modified the CSS file needs to be regenerated, it's better if the website itself automatically runs HSS when it notices that the HSS file has changed, and displays errors if any occurs.

#Syntax
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

#Property Variables
HSS adds property variables to the CSS syntax.

You can declare a variable with the keyword var and use it in the HSS file by prefixing the variable name with a dollar :
```
var mycolor = #1111AA;
var myfont = "Trebuchet MS", Arial, sans-serif;

body {
    color : $mycolor;
    font : $myfont;
}
```
Variables are a very good way to avoid replacing everything in a CSS file every time you want to change a single color value. It also helps designers to remember and easily locate the main colors that are used in a given website.

#Block Variables
You can also declare variables that will declare several properties at the same time :
```
var nomargin = { margin : 0px; padding : 0px; }

pre {
    $nomargin;
    color : #FF0000;
}
```
Block variables are very useful to share some "behaviors" between unrelated rules. They can be used to improve the way CSS files are organized and makes it much more easy to test style-changes.

#Nested Blocks
One of the things that are the most annoying with CSS is when you want to avoid class-names clashing. For instance if you declare the following :
```
.name { font-weight : bold; }
```
Then there is an high possibility that class="name" is used in several different parts of the website, and needs to be styled differently depending on the case.

In order to avoid these problems, it's often needed to specify the context in which is class occurs, which often to the following declarations :
```
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
```
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

#Comments
There are two kind of comments possible in HSS, which are the same used in popular syntax such as C/Java. The `/* ... */` comment is a multiline comment which is already part of CSS and the `// ....` comment is used for single-line comment.
```
p {
//  color : red;
    font-weight : bold;
/*  margin : 0px;
    padding : 0px; */    
}
```
All comments are removed from the CSS output.

#CSS Validation
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
 * the background property has a fixed order, which is the following : 
 * background : [color] [url] [repeat] [scroll|fixed] [horiz.] [vert.]
 
#CSS Rules
The whole CSS properties rules that are used to check the property value correctness are listed in the Rules.nml file of the HSS sources. You might want to modify them for your own needs.

#Selectors
Here's the list of CSS selectors with their HSS support status, see here for the corresponding explanations :
```
E : supported
E F : supported
E > F : supported
E + F : supported
E ~ F : supported
DIV.warning : supported
E#myid : supported
E:first-child, E:link, E:hover and other CSS2 and 3 pseudo-classes selectors are supported (except the ones that takes an argument
E[foo], E[foo=value], E[foo|=value], E[foo~=value], E[foo^=value], E[foo$=value], E[foo*=value] : supported
```
Other selectors are not supported, but can be used with the CSS function :
```
.page CSS("h1.nth-child(2)") {
    color : red
}
```
#IE hacks
It's sometimes useful to use non-standard CSS properties, in particular for various IE-specific things. HSS adds a specific command for doing that. Here's a small example how it can be used :
```
.image {
    my-special-property : CSS("some specific value");
}
```
In that case, this will simply output the property in the CSS file without checking that it's defined in the CSS standard or that the value is correct.

#Operations
It is possible to perform some operations between values :
```
var scale = 3;
.big {
    width : 50px + 20px;
    height : 30px * $scale;
    color : #FF0000 * 0.7;
}
```
Operations between two different units (for instance 50px + 3em) are not allowed.

#Hacks Support
Some hacks has been added to support new CSS properties on most recent browsers.

 * `opacity` : will also generate the corresponding IE filter alpha
 * `border-radius` (and border-top-left-radius, etc) : will also generate -moz-border-radius and -webkit-border-radius for Firefox and Chrome/Safari support
 * `box-shadow` : will also generate both -moz-box-shadow and -webkit-box-shadow for Firefor and Chrome/Safari support
 * `text-shadow` : supported by HSS, but doesn't show anything on IE6-8
 * `display` : inline-block : will add a IE fix to get correct support
 * `color : rgba(r,g,b,a)` : will add a solid color default value for browsers which don't support rgba
 * `background-color : rgba(r,g,b,a)` : will add a solid color default value for browsers which don't support rgba, and also add some IE6-8 hacks that correctly display the transparent background color. The only drawback is that the block background image will display below the background color.
 * `hss-width` and `hss-height` : will generate width and height from which will be subtracted the padding and border values declared in the current block.
 * `background : linear-gradient(#color1,#color2)` : will add support for all browsers, including IE6-8 New in 1.4
 * You can also add `@include('some css string')` either at the top level or instead of an attribute, this will include raw CSS string in the output, prefixed with the hierarchy classes if any New in 1.4

#Credits
The HSS software was developed by Nicolas Cannasse for Motion-Twin. 
