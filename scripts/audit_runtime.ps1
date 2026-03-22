$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$stdRoot = Join-Path $repoRoot "std"
$spearcPath = Join-Path $repoRoot "src\spearc.c"

$stdFunctions = Get-ChildItem -Path $stdRoot -Recurse -Filter *.sharp |
    Select-String -Pattern 'python_json\("spear_std", "([^"]+)"' |
    ForEach-Object { $_.Matches[0].Groups[1].Value } |
    Sort-Object -Unique

$nativeFunctions = Get-Content $spearcPath |
    Select-String -Pattern 'if \(strcmp\(fn, \\"([^"]+)\\"\) == 0\) return spear_native_' |
    ForEach-Object { $_.Matches[0].Groups[1].Value } |
    Sort-Object -Unique

if ((Get-Content $spearcPath | Select-String -Quiet -Pattern 'strcmp\(fn, \\"safe_http_request\\"\) == 0')) {
    $nativeFunctions = @($nativeFunctions + "safe_http_request") | Sort-Object -Unique
}

$missing = $stdFunctions | Where-Object { $_ -notin $nativeFunctions }
$covered = $stdFunctions | Where-Object { $_ -in $nativeFunctions }

Write-Output "Spear Runtime Audit"
Write-Output "==================="
Write-Output ("std spear_std calls : {0}" -f $stdFunctions.Count)
Write-Output ("native intercepts   : {0}" -f $covered.Count)
Write-Output ("remaining fallback  : {0}" -f $missing.Count)
Write-Output ""

if ($missing.Count -gt 0) {
    Write-Output "Remaining fallback functions:"
    $missing | ForEach-Object { Write-Output ("- {0}" -f $_) }
} else {
    Write-Output "All std spear_std calls are covered by native intercepts."
}

