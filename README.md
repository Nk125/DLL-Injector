# DLL-Injector
Simple DLL injector (**CreateRemoteThread** method) with some examples

## Config

In CONFIG.h you can setup the path to a module or example to compile in DLL

Also you can set other options like:
  * Parameters
  * Executable Path
  * DLL Path
and others

## Writing custom modules

You can create your own module in C++ freely!

If you want, you can fork this and make a pull request to add your own module

The first thing to take in account is the **MAIN function**, in any module, this is needed to run it succesfully into the DLL

The MAIN function takes only 1 mandatory argument, the HINSTANCE of the DLL, you can ignore it, but it's necessary to do other things with WinAPI

It's required to include windows.h in the module to acquire the definition of HINSTANCE in the MAIN function

### Finally

This is made with educational purposes, without any warranty if anyone/anything is damaged by outsiders of this project.
