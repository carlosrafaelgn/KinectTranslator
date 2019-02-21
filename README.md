KinectTranslator
================

During Scratch Conference 2012 (Scratch@MIT 2012 | Create Your World), held at MIT in July, I was introduced to an amazing tool which allows Scratch to connect to the Microsoft Kinect Sensor Bar, called Kinect 2 Scratch (http://scratch.saorog.com and http://channel9.msdn.com/coding4fun/kinect/Kinect-2-Scratch).

Even though it is such an amazing tool, it was not possible to find some of the features I truly wanted:

- Capability to select the values sent to Scratch, considerably shrinking the displayed list of sensors;
- A light-weight UI, making it easier for low-end computers to run the software;
- Being able to translate the UI into other languages without having to recompile the software.

Kinect Translator has been created with all that in mind. The software has no main window, as it runs in the background, being controlled by its menu commands, accessed through the icon at the system tray.

The default UI language is English, but once a configuration file is created, along with a Unicode translation file, it is possible to change the language (the committed configuration file sets the language to Portuguese).

In order to execute this program, you will need [Kinect for Windows Runtime v1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40277) (you can try to download and install [Kinect for Windows Runtime v1.5](http://go.microsoft.com/fwlink/?LinkId=253187) for older Windows versions).

If you plan to rebuild this source code or if you intend to move further into Kinect development for Windows, you should skip downloading Kinect for Windows Runtime and should install [Kinect for Windows SDK v1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40278) instead, which already comes with all necessary runtime files.

For more information, check out the website http://carlosrafaelgn.com.br/Scratch (in Portuguese, for now...)

This projected is licensed under the terms of the FreeBSD License. See LICENSE.txt for more details.
