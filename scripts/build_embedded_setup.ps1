param(
    [Parameter(Mandatory = $true)][string] $InputExe,
    [Parameter(Mandatory = $true)][string] $SourceDir,
    [Parameter(Mandatory = $true)][string] $OutputExe
)

$ErrorActionPreference = "Stop"

function Write-U32([System.IO.BinaryWriter] $writer, [uint32] $value) {
    $writer.Write([BitConverter]::GetBytes($value))
}

function Write-U64([System.IO.BinaryWriter] $writer, [uint64] $value) {
    $writer.Write([BitConverter]::GetBytes($value))
}

function Get-RelativeUnixPath([string] $basePath, [string] $fullPath) {
    $normalizedBase = [System.IO.Path]::GetFullPath($basePath)
    if (-not $normalizedBase.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
        $normalizedBase += [System.IO.Path]::DirectorySeparatorChar
    }
    $normalizedFull = [System.IO.Path]::GetFullPath($fullPath)
    if ($normalizedFull.StartsWith($normalizedBase, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $normalizedFull.Substring($normalizedBase.Length).Replace("\", "/")
    }
    throw "File '$fullPath' is not under '$basePath'."
}

$inputPath = (Resolve-Path $InputExe).Path
$sourcePath = (Resolve-Path $SourceDir).Path
$outputPath = [System.IO.Path]::GetFullPath($OutputExe)
$outputDir = Split-Path -Parent $outputPath
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

$payloadPath = Join-Path ([System.IO.Path]::GetTempPath()) ("spear-payload-" + [Guid]::NewGuid().ToString("N") + ".bin")
$headerMagic = [System.Text.Encoding]::ASCII.GetBytes("SPRPKG01")
$trailerMagic = [System.Text.Encoding]::ASCII.GetBytes("SPREMB01")

try {
    $files = Get-ChildItem -Path $sourcePath -Recurse -File | Sort-Object FullName
    $payloadStream = [System.IO.File]::Create($payloadPath)
    try {
        $writer = New-Object System.IO.BinaryWriter($payloadStream, [System.Text.Encoding]::UTF8, $true)
        $writer.Write($headerMagic)
        Write-U32 $writer 1
        Write-U32 $writer ([uint32] $files.Count)
        foreach ($file in $files) {
            $relative = Get-RelativeUnixPath $sourcePath $file.FullName
            $relativeBytes = [System.Text.Encoding]::UTF8.GetBytes($relative)
            Write-U32 $writer ([uint32] $relativeBytes.Length)
            Write-U64 $writer ([uint64] $file.Length)
            $writer.Write($relativeBytes)

            $inStream = $file.OpenRead()
            try {
                $buffer = New-Object byte[] 65536
                while (($read = $inStream.Read($buffer, 0, $buffer.Length)) -gt 0) {
                    $writer.Write($buffer, 0, $read)
                }
            }
            finally {
                $inStream.Dispose()
            }
        }
        $writer.Flush()
    }
    finally {
        $payloadStream.Dispose()
    }

    $payloadSize = (Get-Item $payloadPath).Length
    $exeSize = (Get-Item $inputPath).Length
    $outStream = [System.IO.File]::Create($outputPath)
    try {
        foreach ($path in @($inputPath, $payloadPath)) {
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
        Write-U64 $trailer ([uint64] $payloadSize)
        $trailer.Flush()
    }
    finally {
        $outStream.Dispose()
    }
}
finally {
    if (Test-Path $payloadPath) {
        Remove-Item -Force $payloadPath
    }
}
