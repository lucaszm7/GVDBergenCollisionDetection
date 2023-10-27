# Install Dependencies
[CmdletBinding()]
param (
    [Parameter()][Boolean] $isDLL=$true,
    [Parameter()][String]  $Configuration="Release"
)

Write-Host "[LOG] Building GVDB Collision Detection..." -f Blue;
if($isDLL) 
{
    Write-Host "[LOG] Generating DLL" -f Blue;
}
else
{
    Write-Host "[LOG] Generating STATIC LIB" -f Blue;
}

# Build Solution
$solutionType = If ($Configuration.equals("Release")) { "Rel_LIB" } Else { "Deb_LIB" };
if ($isDLL) {$solutionType = "Release";}

Write-Host "[LOG] Building Solution $solutionType ..." -f Blue;
msbuild GVDBergenCollisionDetection.sln /p:Configuration=$solutionType;
if($?)
{
    Write-Host "[LOG] Solution Builded!" -f Green;
    if($isDLL)
        { Write-Host "[LOG] DLL is in bin\Release\x64\GVDBergenCollisionDetection.dll" -f Green; }
    else
        { Write-Host "[LOG] Lib is in bin\$solutionType\x64\GVDBergenCollisionDetection.lib" -f Green; }
}
else 
{
    Write-Host "[ERROR] Could not build solution!" -f Red;
    Write-Host "[HINT] This script has to be executed in PowerShell for Visual Studio" -f Blue;
    $PWD;
    exit 1;
}

