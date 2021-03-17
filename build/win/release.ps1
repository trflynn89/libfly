param (
    [Parameter(Mandatory=$true)][string]$arch
)

# Create a release package for an architecture.
function Create-Libfly-Package($arch)
{
    Write-Output "Creating $arch package"

    $version = Get-Content -Path ($PSScriptRoot + "\..\..\VERSION.md")

    $package_src_path = $PSScriptRoot + "\..\..\fly"
    $package_lib_path = $PSScriptRoot + "\..\Release\msvc\" + $arch + "\libfly"
    $package_tmp_path = $PSScriptRoot + "\..\Release\msvc\libfly-win-" + $version + "." + $arch
    $package_zip_path = $package_tmp_path + ".zip"

    Remove-Item -Path $package_tmp_path -Recurse -ErrorAction SilentlyContinue
    Remove-Item -Path $package_zip_path -ErrorAction SilentlyContinue
    New-Item -Path $package_tmp_path -ItemType Directory

    Copy-Item -Path ($package_lib_path + "\libfly." + $arch + ".lib") -Destination $package_tmp_path
    Copy-Item -Path $package_src_path -Destination $package_tmp_path -Recurse -Exclude @("*.mk", "*.cpp")

    Compress-Archive -Path ($package_tmp_path + "\*") -Destination $package_zip_path -CompressionLevel Optimal
    Remove-Item -Path $package_tmp_path -Recurse -ErrorAction SilentlyContinue
}

# Create the package
Create-Libfly-Package $arch
