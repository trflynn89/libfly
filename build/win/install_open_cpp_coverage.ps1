$version = "0.9.7.0"

$installer_url = "https://github.com/OpenCppCoverage/OpenCppCoverage/releases/download/release-" + $version + "/OpenCppCoverageSetup-x64-" + $version + ".exe"
$installer_path = ${Env:USERPROFILE} + "\Downloads\OpenCppCoverageSetup.exe"

Write-Host -ForegroundColor White ("Downloading OpenCppCoverage: " + $installer_url)
(New-Object System.Net.WebClient).DownloadFile($installer_url, $installer_path)

Write-Host -ForegroundColor White "Installing OpenCppCoverage"
$install = (Start-Process $installer_path -ArgumentList '/VERYSILENT' -PassThru -Wait)

if ($install.ExitCode -ne 0)
{
    throw "Failed to install OpenCppCoverage"
}
