# Run all unit tests for an architecture and upload results to appveyor.
function Run-Libfly-Test($arch)
{
    Write-Output "Running $arch tests"
    $full_path = $PSScriptRoot + "\\Debug-" + $arch

    Get-ChildItem -path $full_path -Recurse -Include *.exe  | ForEach {
        $test = $_.Directory.Name
        & $_

        if ($LASTEXITCODE)
        {
            Add-AppveyorTest $test -Outcome Failed -FileName $_ -ErrorMessage "Failed $test test"
        }
        else
        {
            Add-AppveyorTest $test -Outcome Passed -FileName $_
        }
    }
}

# Run the tests
Run-Libfly-Test x86
Run-Libfly-Test x64
