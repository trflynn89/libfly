# Run all unit tests for an architecture and upload results to appveyor.
function Run-Libfly-Test($arch)
{
    Write-Output "Running $arch tests"

    $full_path = $PSScriptRoot + "\\Debug-" + $arch
    $tests_passed = 0
    $tests_failed = 0

    Get-ChildItem -path $full_path -Recurse -Include *_test.exe  | ForEach {
        $timer = [Diagnostics.Stopwatch]::StartNew()
        & $_ 2>&1 | tee -Variable output
        $status = $LASTEXITCODE
        $timer.Stop()

        $duration = $timer.Elapsed.TotalMilliseconds
        $test = $_.Directory.Name + "_" + $arch

        $output = Extract-Stdout-Stderr($output)
        $stdout = $output[0]
        $stderr = $output[1]

        if ($status -eq 0)
        {
            Add-AppveyorTest $test -Outcome Passed -FileName $_ -StdOut $stdout -StdErr $stderr -Duration $duration
            ++$tests_passed
        }
        else
        {
            Add-AppveyorTest $test -Outcome Failed -FileName $_ -StdOut $stdout -StdErr $stderr -Duration $duration -ErrorMessage "Failed $test test"
            ++$tests_failed
        }
    }

    if (($tests_passed -eq 0) -or ($tests_failed -ne 0))
    {
        Write-Error "Failed $arch tests"
        exit 1
    }
}

function Extract-Stdout-Stderr($output)
{
    $stdout = $output | ?{ $_ -isnot [System.Management.Automation.ErrorRecord] }
    $stderr = $output | ?{ $_ -is [System.Management.Automation.ErrorRecord] }

    if ([string]::IsNullOrEmpty($stdout))
    {
        $stdout = (Out-String).Trim()
    }
    else
    {
        $stdout = ($stdout | Out-String).Trim()
    }

    if ([string]::IsNullOrEmpty($stderr))
    {
        $stderr = (Out-String).Trim()
    }
    else
    {
        $stderr = ($stderr | Out-String).Trim()
    }

    $stdout
    $stderr
}

# Run the tests
Run-Libfly-Test x86
Run-Libfly-Test x64
