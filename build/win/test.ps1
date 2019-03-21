param (
    [Parameter(Mandatory=$true)][string]$arch
)

# Run all unit tests for an architecture.
function Run-Libfly-Test($arch)
{
    Write-Output "Running $arch tests"

    $full_path = $PSScriptRoot + "\Debug-" + $arch
    $tests_passed = 0
    $tests_failed = 0

    Get-ChildItem -path $full_path -Recurse -Include *_tests.exe  | ForEach {
        & $_

        if ($LASTEXITCODE -eq 0)
        {
            ++$tests_passed
        }
        else
        {
            ++$tests_failed
        }
    }

    if (($tests_passed -eq 0) -or ($tests_failed -ne 0))
    {
        Write-Error "Failed $arch tests"
        exit 1
    }
}

# Run the tests
Run-Libfly-Test $arch
