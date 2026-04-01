# Axios npm Supply Chain Attack - Compromise Detection (Windows Only)

Detection tools for the March 31, 2026 axios npm hijacking incident where malicious versions 1.14.1 and 0.30.4 were distributed containing a hidden RAT (Remote Access Trojan). **Windows-only implementation.**

## The Attack

**Timeline**: March 31, 2026, ~2-3 hours
**Affected Versions**: axios@1.14.1 (1.x branch), axios@0.30.4 (0.x branch)
**Attack Method**: Account hijacking of `jasonsaayman` npm account
**Payload Mechanism**: Hidden dependency `plain-crypto-js@4.2.1` with malicious `postinstall` script

### What the Malware Does

1. **Initial Contact**: RAT contacts `sfrclak.com:8000/6202033` within 2 seconds of installation
2. **Windows Payload**: Copies PowerShell to `%PROGRAMDATA%\wt.exe`, writes VBScript dropper to `%TEMP%\6202033.vbs` and PowerShell stage-2 to `%TEMP%\6202033.ps1`
3. **Anti-Forensics**: Self-destructs by deleting malicious `package.json` and replacing with clean v4.2.0 stub

## Quick Start

```cmd
REM Check current directory
node axios-compromise-detector.js

REM Scan specific directory
node axios-compromise-detector.js --scan-dir C:\path\to\project

REM Check network connections (requires admin)
node axios-compromise-detector.js --check-network

REM Output as JSON
node axios-compromise-detector.js --json
```

## What Gets Checked

### 1. Vulnerable Package Versions
- Scans `package.json` for vulnerable axios versions (1.14.1, 0.30.4)
- Detects `plain-crypto-js` in `package.json` dependencies (phantom dependency — never imported by axios, high-confidence indicator)
- Checks for malicious `plain-crypto-js@4.2.1` in `node_modules`
- Detects version-spoofed `plain-crypto-js@4.2.0` (dropper replaces 4.2.1 with a clean 4.2.0 stub to hide evidence)
- **Confidence**: 30-50% per finding

### 2. Persistence & Dropper Artifacts (Windows)
- `%PROGRAMDATA%\wt.exe`
- `%TEMP%\6202033.vbs`
- `%TEMP%\6202033.ps1`
- **Confidence**: 35% per finding

### 3. Network Connections (with `--check-network`)
- Detects active connections to `sfrclak.com`
- Detects connections to C2 IP: `142.11.206.73:8000`
- **Confidence**: 50% per finding
- **Requires**: Admin/elevated privileges

## Output

### Text Report (Default)
```
======================================================================
AXIOS COMPROMISE DETECTION REPORT
======================================================================
Timestamp: 2026-04-01T10:30:45.123Z
Platform: Windows
Compromised: YES
Severity: CRITICAL
Confidence: 85%
======================================================================

⚠ Found 2 indicator(s) of compromise:

[1] VULNERABLE_VERSION [CRITICAL]
    Message: Found vulnerable axios version 1.14.1 in C:\app\package.json
    Component: axios
    Version: 1.14.1
    Action: Update axios immediately: npm install axios@latest

[2] MALICIOUS_PACKAGE_PRESENT [CRITICAL]
    Message: Found malicious plain-crypto-js@4.2.1 in node_modules
    Path: C:\app\node_modules\plain-crypto-js
    Component: plain-crypto-js
    Version: 4.2.1
    Action: Remove node_modules and reinstall: rm -rf node_modules package-lock.json && npm install

======================================================================
```

### JSON Output
```cmd
node axios-compromise-detector.js --json
```

Returns structured JSON for programmatic processing.

## Severity Levels & Remediation

| Finding | Severity | Action | Timeline |
|---------|----------|--------|----------|
| Vulnerable axios version in package.json | CRITICAL | Update immediately | NOW |
| plain-crypto-js in package.json (phantom dep) | CRITICAL | Remove dependency, reinstall | NOW |
| plain-crypto-js@4.2.1 in node_modules | CRITICAL | Reinstall dependencies | NOW |
| plain-crypto-js@4.2.0 version spoof | HIGH | Reinstall dependencies | NOW |
| Persistence/dropper files detected | CRITICAL | Delete files, run antivirus | NOW |
| C2 connections active | CRITICAL | Isolate machine, investigate | IMMEDIATELY |

## Critical Remediation Steps

If compromise is detected:

### Step 1: Isolate (If C2 Connection Found)
```cmd
REM Disconnect network or block outbound traffic
REM Do NOT shut down yet (may alert attacker)
```

### Step 2: Update Vulnerable Code
```cmd
npm uninstall axios plain-crypto-js
rmdir /s /q node_modules
del package-lock.json
npm ci
npm install axios@latest
```

### Step 3: Remove Persistence & Dropper Artifacts
```cmd
REM Run as Administrator
del "%PROGRAMDATA%\wt.exe"
del "%TEMP%\6202033.vbs"
del "%TEMP%\6202033.ps1"
```

### Step 4: Investigate & Scan
- Review git history for when axios was last updated
- Run antivirus/EDR full system scan
- Check logs for network connections to 142.11.206.73:8000
- Check for unusual spawned processes or scheduled tasks

### Step 5: Audit Credentials
If compromise confirmed:
- Rotate all credentials (API keys, SSH keys, passwords)
- Revoke active sessions
- Audit access logs for lateral movement
- Consider assuming all secrets may be compromised

## For CI/CD Pipelines

### GitHub Actions
```yaml
- name: Check for axios compromise
  run: |
    node axios-compromise-detector.js --json > report.json
    if [ $? -eq 1 ]; then
      echo "Compromised version detected!"
      cat report.json
      exit 1
    fi
```

### GitLab CI
```yaml
axios_check:
  script:
    - node axios-compromise-detector.js --json
  allow_failure: false
```

### Pre-commit Hook
```bash
#!/bin/bash
node axios-compromise-detector.js --scan-dir .
if [ $? -eq 1 ]; then
  echo "ERROR: Axios compromise detected in dependencies"
  exit 1
fi
```

## Advanced Usage

### Scan Multiple Projects
```cmd
for /d %%D in (C:\projects\*) do (
  echo Scanning %%D...
  node axios-compromise-detector.js --scan-dir "%%D" --json >> results.json
)
```

### Generate Audit Report
```cmd
node axios-compromise-detector.js --check-network > audit_%date:~-4,4%%date:~-10,2%%date:~-7,2%.txt
```

## False Positives

- Tools report findings based on file presence, package versions, and network patterns
- Legitimate software may create similarly-named files (rare)
- Corporate proxies or VPNs may show unusual network patterns

Always investigate findings rather than ignoring them.

## Limitations

1. **Network checks require elevated privileges** (admin)
2. If RAT has already self-destructed, `node_modules/plain-crypto-js` may be gone
3. Persistence files may have been deleted by antivirus
4. C2 server may be down or IP may have been reclaimed

If your package.json shows the vulnerable version, **you were potentially compromised** regardless of whether runtime artifacts remain.

## References

- StepSecurity Blog: [Axios Compromised on npm](https://www.stepsecurity.io/blog/axios-compromised-on-npm-malicious-versions-drop-remote-access-trojan)
- npm Security Advisory: axios 1.14.1, 0.30.4
- C2 Infrastructure: `sfrclak.com` (142.11.206.73:8000)

## Support

For questions or to report detection issues:
1. Run with `--json` flag to get structured output
2. Include platform info: `node -v`, `npm -v`
3. Include full JSON report from detection tool
