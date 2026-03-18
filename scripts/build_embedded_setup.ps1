param(
    [Parameter(Mandatory = $true)][string] $InputExe,
    [Parameter(Mandatory = $true)][string] $SourceDir,
    [Parameter(Mandatory = $true)][string] $OutputExe
)

$ErrorActionPreference = "Stop"

function Write-U64([System.IO.BinaryWriter] $writer, [uint64] $value) {
    $writer.Write([BitConverter]::GetBytes($value))
}

$inputPath = (Resolve-Path $InputExe).Path
$sourcePath = (Resolve-Path $SourceDir).Path
$outputPath = [System.IO.Path]::GetFullPath($OutputExe)
$outputDir = Split-Path -Parent $outputPath
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

$zipPath = Join-Path ([System.IO.Path]::GetTempPath()) ("spear-payload-" + [Guid]::NewGuid().ToString("N") + ".zip")
$trailerMagic = [System.Text.Encoding]::ASCII.GetBytes("SPREMB01")

try {
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    if (Test-Path $zipPath) {
        Remove-Item -Force $zipPath
    }
    [System.IO.Compression.ZipFile]::CreateFromDirectory($sourcePath, $zipPath, [System.IO.Compression.CompressionLevel]::Optimal, $false)

    $zipSize = (Get-Item $zipPath).Length
    $exeSize = (Get-Item $inputPath).Length
    $outStream = [System.IO.File]::Create($outputPath)
    try {
        foreach ($path in @($inputPath, $zipPath)) {
            $inStream = [System.IO.File]::OpenRead($path)
            try {
                $buffer = New-Object byte[] 65536
                while (($read = $inStream.Read($buffer, 0, $buffer.Length)) -gt 0) {
                    $outStream.Write($buffer, 0, $read)
                }
            }
            finally {
                $inStream.Dispose()
            }
        }

        $trailer = New-Object System.IO.BinaryWriter($outStream, [System.Text.Encoding]::ASCII, $true)
        $trailer.Write($trailerMagic)
        Write-U64 $trailer ([uint64] $exeSize)
        Write-U64 $trailer ([uint64] $zipSize)
        $trailer.Flush()
    }
    finally {
        $outStream.Dispose()
    }
}
finally {
    if (Test-Path $zipPath) {
        Remove-Item -Force $zipPath
    }
}
