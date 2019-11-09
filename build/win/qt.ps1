$QT_INSTALLER = "qt-unified-windows-x86-online.exe"
$QT_INSTALLER_URL = "http://qt.mirror.constant.com/official_releases/online_installers/" + $QT_INSTALLER
$QT_INSTALLER_PATH = ${Env:USERPROFILE} + "\Downloads\" + $QT_INSTALLER
$QT_INSTALLER_SCRIPT = $PSScriptRoot + "\..\ci\qt.js"

# Delete a path regardless of whether it is a directory or file.
function Remove-Path($path)
{
    if (Test-Path -Path $path -PathType Container)
    {
        Get-ChildItem $path -Force -Recurse | Remove-Item -Force -Recurse
        Remove-Item -Force -Recurse $path
    }
    elseif (Test-Path -Path $path -PathType Leaf)
    {
        Remove-Item -Force $path
    }
}

# Read the assignment of a variable from the Qt installer script.
function Read-Qt-Variable($variable)
{
    $regex = "$variable = '(.*)'"
    $candidates = Select-String -Pattern $regex -Path $QT_INSTALLER_SCRIPT

    return [regex]::unescape($candidates.Matches.Groups[1].Value)
}

# Download the Qt installer executable.
function Download-Qt()
{
    Write-Output ("Downloading Qt installer: " + $QT_INSTALLER_URL)
    Remove-Path $QT_INSTALLER_PATH

    $installer = New-Object System.Net.WebClient
    $installer.DownloadFile($QT_INSTALLER_URL, $QT_INSTALLER_PATH)
}

# Uninstall any exisiting Qt installation and (re)install Qt.
function Install-Qt()
{
    Write-Output "Installing Qt"

    $qt_install_point = Read-Qt-Variable "QT_INSTALL_POINT_WINDOWS"
    Remove-Path $qt_install_point

    $arguments = '--script',$QT_INSTALLER_SCRIPT
    Start-Process $QT_INSTALLER_PATH -ArgumentList $arguments -NoNewWindow -Wait

    Remove-Path $QT_INSTALLER_PATH
}

Download-Qt
Install-Qt
