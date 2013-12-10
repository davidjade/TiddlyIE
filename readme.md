TiddlyIE
========

Internet Explorer extension for TiddlyWiki 5


This is an Internet Explorer Extension (Browser Helper Object or BHO) that enables TiddyWiki 5 to safely save Wiki content to the local file system.

To build this project you will need Visual Studio 2010 or later.

To have the project build automatically register the extension you will have to run VS with elevated priledges. Alternatively, you can manually register the extension by executing:

	regsvr32 TiddlyIEBHO.dll


Setup project
=============

The Setup project builds two targets; an x86 and x64 installer.



License
=======


TiddyIE is available under the BSD license. See the License.rft file for more info.

Copyright &copy; 2013 TinyOctopus LLC