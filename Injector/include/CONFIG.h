#pragma once
// DLL CONFIG
#define MAINPROCPATH "Procedures\\msgbox.hpp" /*
 The path to a hpp path with the MAIN function
*/

// INJECTOR CONFIG
#define POPEN 0 /*
 Type of process open
 0: CreateProcessA Process HANDLE, 1: OpenProcess with PID
 If you don't know what are these, set POPEN to 0
 */

#define EXEPATH "C:\\Windows\\System32\\svchost.exe" /*
 Executable FULL path
 */

#define CDIR "C:\\Windows\\System32" /*
 Current dir for the injected process
 */

#define DLLPATH "DLL.dll" /*
 Dll path to inject, note that if the dll is renamed, the process can't be injected
 */

#define PARAMS "" /*
 Parameters passed to the process
 */

#define AWAIT 0 /*
 Define if you want to wait until the code inside DLL is finished, if you have a blocking
 socket or function, deactivate this.
 1: Await, 0: Don't await
 */
