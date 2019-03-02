@echo off

:: Create release zips
call:create_release x86
call:create_release x64
exit /B 0

:: Create a release zip (expects x86 or x64 as input)
:create_release
    echo Creating %1 release

    set /p fly_version= < %root_dir%..\..\VERSION.md
    set fly_src_dir=%root_dir%..\..\fly
    set fly_rel_dir=%root_dir%libfly-win-%fly_version%.%1
    set fly_rel_zip=%fly_rel_dir%.zip
    set fly_exclude=%root_dir%exclude.txt

    echo .mk > %fly_exclude%
    echo .cpp >> %fly_exclude%

    del /Q %fly_rel_zip% 2> NUL
    rmdir /S /Q %fly_rel_dir% 2> NUL
    mkdir %fly_rel_dir%\fly

    copy /V /Y %root_dir%Debug-%1\libfly\libflyd.%1.lib %fly_rel_dir%
    copy /V /Y %root_dir%Release-%1\libfly\libfly.%1.lib %fly_rel_dir%
    xcopy /S /V /Y /Q /EXCLUDE:%fly_exclude% %fly_src_dir% %fly_rel_dir%\fly

    powershell -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('%fly_rel_dir%', '%fly_rel_zip%'); }"
    rmdir /S /Q %fly_rel_dir%
    del /F /Q %fly_exclude%

    exit /B /0
