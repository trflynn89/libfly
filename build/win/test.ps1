# Run all unit tests for an architecture and upload results to appveyor.
function Run-Libfly-Test($arch)
{
    Write-Output "Running $arch tests"

    $full_path = $PSScriptRoot + "\\Debug-" + $arch
    $failed = $FALSE

    Get-ChildItem -path $full_path -Recurse -Include *.exe  | ForEach {
        $timer = [Diagnostics.Stopwatch]::StartNew()
        & $_ 2>&1 | tee -Variable output
        $status = $LASTEXITCODE
        $timer.Stop()

        $duration = $timer.Elapsed.TotalMilliseconds
        $test = $_.Directory.Name + "_" + $arch

        $output = Extract-Stdout-Stderr($output)
        $stdout = $output[0]
        $stderr = $output[1]

        if ($status)
        {
            Add-AppveyorTest $test -Outcome Failed -FileName $_ -StdOut $stdout -StdErr $stderr -Duration $duration -ErrorMessage "Failed $test test"
            $failed = $TRUE
        }
        else
        {
            Add-AppveyorTest $test -Outcome Passed -FileName $_ -StdOut $stdout -StdErr $stderr -Duration $duration
        }
    }

    if ($failed -eq $TRUE)
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
