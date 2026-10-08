$app = "firmware/main-node-f446/App"
$inc = @("-I$app", "-I$app/cli", "-I$app/crc", "-I$app/findblank", "-I$app/health", "-I$app/record", "-I$app/ringbuf")
$tests = @(
    @("cli/test_cli", "cli/cli"),
    @("cli/test_cli_dispatch", "cli/cli"),
    @("cli/test_cli_line", "cli/cli"),
    @("crc/test_crc32", "crc/crc32"),
    @("findblank/test_find_blank", "findblank/find_blank"),
    @("health/test_health", "health/health"),
    @("record/test_record", "record/record", "crc/crc32"),
    @("ringbuf/test_ringbuf", "ringbuf/ringbuf")
)
$failed = 0
foreach ($t in $tests) {
    $name = Split-Path $t[0] -Leaf
    $files = @($t | ForEach-Object { "$app/$_.c" })
    gcc @inc @files -o "local/$name.exe"
    if ($LASTEXITCODE -ne 0) { Write-Host "BUILD FAIL $name"; $failed++; continue }
    $out = & "local/$name.exe" | Out-String
    if (($LASTEXITCODE -ne 0) -or ($out -cmatch "FAIL")) { Write-Host "FAIL $name"; $failed++ } else { Write-Host "PASS $name" }
}
Write-Host "failed: $failed"
exit $failed