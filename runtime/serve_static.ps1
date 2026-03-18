param(
    [Parameter(Mandatory = $true)]
    [string]$Root,
    [int]$Port = 4173
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $Root)) {
    Write-Error "Root not found: $Root"
    exit 1
}

$listener = [System.Net.HttpListener]::new()
$prefix = "http://127.0.0.1:$Port/"
$listener.Prefixes.Add($prefix)
$listener.Start()

try {
    while ($true) {
        $context = $listener.GetContext()
        $requestPath = $context.Request.Url.AbsolutePath.TrimStart("/")
        if ([string]::IsNullOrWhiteSpace($requestPath)) {
            $requestPath = "index.html"
        }
        $requestPath = $requestPath.Replace("/", "\")
        $fullPath = Join-Path $Root $requestPath

        if ((Test-Path -LiteralPath $fullPath) -and -not (Get-Item -LiteralPath $fullPath).PSIsContainer) {
            $ext = [System.IO.Path]::GetExtension($fullPath).ToLowerInvariant()
            switch ($ext) {
                ".html" { $contentType = "text/html; charset=utf-8" }
                ".css"  { $contentType = "text/css; charset=utf-8" }
                ".js"   { $contentType = "application/javascript; charset=utf-8" }
                ".json" { $contentType = "application/json; charset=utf-8" }
                ".svg"  { $contentType = "image/svg+xml" }
                ".png"  { $contentType = "image/png" }
                ".jpg"  { $contentType = "image/jpeg" }
                ".jpeg" { $contentType = "image/jpeg" }
                ".gif"  { $contentType = "image/gif" }
                default { $contentType = "application/octet-stream" }
            }

            $bytes = [System.IO.File]::ReadAllBytes($fullPath)
            $context.Response.StatusCode = 200
            $context.Response.ContentType = $contentType
            $context.Response.ContentLength64 = $bytes.Length
            $context.Response.OutputStream.Write($bytes, 0, $bytes.Length)
        } else {
            $message = [System.Text.Encoding]::UTF8.GetBytes("Not Found")
            $context.Response.StatusCode = 404
            $context.Response.ContentType = "text/plain; charset=utf-8"
            $context.Response.ContentLength64 = $message.Length
            $context.Response.OutputStream.Write($message, 0, $message.Length)
        }

        $context.Response.OutputStream.Close()
    }
} finally {
    $listener.Stop()
    $listener.Close()
}
