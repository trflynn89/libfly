param (
    [Parameter(Mandatory=$true)][string]$configuration,
    [Parameter(Mandatory=$true)][string]$arch
)

$COVERAGE = ${Env:ProgramFiles} + "\OpenCppCoverage\OpenCppCoverage.exe"

# Run all unit tests for an architecture.
function Run-Libfly-Test($configuration, $arch)
{
    Write-Output "Running $arch tests"

    $source_path = Resolve-Path -Path ($PSScriptRoot + "\..\..\fly")
    $output_path = $PSScriptRoot + "\" + $configuration + "-" + $arch

    $tests_passed = 0
    $tests_failed = 0
    $reports = @()

    Get-ChildItem -path $output_path -Recurse -Include *_tests.exe  | ForEach {
        $report = $output_path + "\coverage\" + $_.Directory.Name + ".bin"
        $reports += $report

        & $COVERAGE --sources $source_path --export_type=binary:$report -- $_

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

    Upload-Full-Test-Report $source_path $reports
}

function Upload-Full-Test-Report($source_path, $reports)
{
    $inputs = @()
    $output = ""

    $reports | ForEach {
        $inputs += "--input_coverage=$_"

        if ([string]::IsNullOrEmpty($output))
        {
            $output = Split-Path $_
        }
    }

    $output += "\coverage.xml"

    & $COVERAGE --export_type=cobertura:$output @inputs
    & codecov --root $source_path --no-color --disable gcov -f $output
}

# Run the tests
Run-Libfly-Test $configuration $arch
