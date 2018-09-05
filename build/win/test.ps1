# Run all unit tests for an architecture and upload results to appveyor.
function Run-Libfly-Test($arch)
{
    Write-Output "Running $arch tests"

    $full_path = $PSScriptRoot + "\\Debug-" + $arch
    $failed = $FALSE

    Get-ChildItem -path $full_path -Recurse -Include *.exe  | ForEach {
        $timer = [Diagnostics.Stopwatch]::StartNew()
        & $_ 2>&1 | tee -Variable output
        $timer.Stop()

        $duration = $timer.Elapsed.TotalMilliseconds
        $test = $_.Directory.Name + "_" + $arch

        if ($LASTEXITCODE)
        {
            $stdout = $output | ?{ $_ -isnot [System.Management.Automation.ErrorRecord] }
            $stderr = $output | ?{ $_ -is [System.Management.Automation.ErrorRecord] }

            Add-AppveyorTest $test -Outcome Failed -FileName $_ -StdOut ($stdout | Out-String).Trim() -StdErr ($stderr | Out-String).Trim() -Duration $duration -ErrorMessage "Failed $test test"
            $failed = $TRUE
        }
        else
        {
            Add-AppveyorTest $test -Outcome Passed -FileName $_ -StdOut ($output | Out-String).Trim() -Duration $duration
        }
    }

    if ($failed -eq $TRUE)
    {
        Write-Error "Failed $arch tests"
        exit 1
    }
}

# Run the tests
Run-Libfly-Test x86
Run-Libfly-Test x64
