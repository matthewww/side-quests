#!/usr/bin/env node

/**
 * Axios npm Supply Chain Attack - Compromise Detection Tool (Windows only)
 *
 * Detects indicators of compromise from the axios 1.14.1/0.30.4 attack (March 31, 2026)
 *
 * Attack Vector:
 * - Malicious versions: axios@1.14.1 and axios@0.30.4
 * - Hidden dependency: plain-crypto-js@4.2.1 with postinstall dropper
 * - RAT contacts: sfrclak.com:8000 (IP: 142.11.206.73)
 *
 * Usage:
 *   node axios-compromise-detector.js [--scan-dir <path>] [--check-network] [--json]
 */

const fs = require('fs');
const path = require('path');
const os = require('os');
const { execSync } = require('child_process');

// Configuration
const CONFIG = {
  // Malicious versions
  COMPROMISED_VERSIONS: {
    'axios': ['1.14.1', '0.30.4'],
    'plain-crypto-js': ['4.2.1']
  },
  // Known C2 infrastructure
  C2_DOMAINS: ['sfrclak.com'],
  C2_IPS: ['142.11.206.73'],
  C2_PORT: 8000,
  // Persistence and dropper artifacts (Windows)
  PERSISTENCE_PATHS: [
    path.join(process.env.PROGRAMDATA || 'C:\\ProgramData', 'wt.exe'),
    path.join(process.env.TEMP || os.tmpdir(), '6202033.vbs'),
    path.join(process.env.TEMP || os.tmpdir(), '6202033.ps1')
  ],
  // Anti-forensics: dropper replaces plain-crypto-js 4.2.1 with 4.2.0 stub
  SPOOFED_VERSIONS: {
    'plain-crypto-js': ['4.2.0']
  }
};

// Results tracking
const results = {
  timestamp: new Date().toISOString(),
  platform: os.platform(),
  findings: [],
  summary: {
    compromised: false,
    severity: 'NONE', // NONE, LOW, MEDIUM, HIGH, CRITICAL
    confidence: 0 // 0-100
  }
};

/**
 * Check for vulnerable axios installation in package.json
 */
function checkAxiosVersion(scanDir = process.cwd()) {
  const packageJsonPath = path.join(scanDir, 'package.json');
  if (!fs.existsSync(packageJsonPath)) {
    return;
  }

  try {
    const pkg = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));
    const axiosVersion = pkg.dependencies?.axios || pkg.devDependencies?.axios;

    if (!axiosVersion) return;

    const version = axiosVersion.replace(/^[\^~>=<\s]+/, '').split(/\s/)[0];

    if (CONFIG.COMPROMISED_VERSIONS.axios.includes(version)) {
      results.findings.push({
        type: 'VULNERABLE_VERSION',
        severity: 'CRITICAL',
        component: 'axios',
        version,
        message: `Found vulnerable axios version ${version} in ${packageJsonPath}`,
        remediation: 'Update axios immediately: npm install axios@latest'
      });
      results.summary.compromised = true;
      results.summary.severity = 'CRITICAL';
      results.summary.confidence += 50;
    }
  } catch (err) {
    console.error(`Error parsing ${packageJsonPath}:`, err.message);
  }
}

/**
 * Check for plain-crypto-js in package.json dependencies (phantom dependency indicator)
 */
function checkPhantomDependency(scanDir = process.cwd()) {
  const packageJsonPath = path.join(scanDir, 'package.json');
  if (!fs.existsSync(packageJsonPath)) return;

  try {
    const pkg = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));
    const hasDep = pkg.dependencies?.['plain-crypto-js'] || pkg.devDependencies?.['plain-crypto-js'];

    if (hasDep) {
      results.findings.push({
        type: 'PHANTOM_DEPENDENCY',
        severity: 'CRITICAL',
        component: 'plain-crypto-js',
        version: hasDep,
        message: `plain-crypto-js listed in package.json but is never imported by axios — high-confidence compromise indicator`,
        remediation: 'Remove plain-crypto-js from dependencies and reinstall: rm -rf node_modules package-lock.json && npm install'
      });
      results.summary.compromised = true;
      results.summary.severity = 'CRITICAL';
      results.summary.confidence += 45;
    }
  } catch (err) {
    // Ignore parse errors; checkAxiosVersion already handles this
  }
}

/**
 * Check for plain-crypto-js malicious package in node_modules
 */
function checkMaliciousDependency(scanDir = process.cwd()) {
  const nodeModulesPath = path.join(scanDir, 'node_modules', 'plain-crypto-js');

  if (fs.existsSync(nodeModulesPath)) {
    try {
      const pkgJsonPath = path.join(nodeModulesPath, 'package.json');
      if (fs.existsSync(pkgJsonPath)) {
        const pkg = JSON.parse(fs.readFileSync(pkgJsonPath, 'utf8'));
        const version = pkg.version;

        if (CONFIG.COMPROMISED_VERSIONS['plain-crypto-js'].includes(version)) {
          results.findings.push({
            type: 'MALICIOUS_PACKAGE_PRESENT',
            severity: 'CRITICAL',
            component: 'plain-crypto-js',
            version,
            path: nodeModulesPath,
            message: `Found malicious plain-crypto-js@${version} in node_modules`,
            remediation: 'Remove node_modules and reinstall: rm -rf node_modules package-lock.json && npm install'
          });
          results.summary.compromised = true;
          results.summary.severity = 'CRITICAL';
          results.summary.confidence += 40;
        } else if (CONFIG.SPOOFED_VERSIONS['plain-crypto-js'].includes(version)) {
          // Anti-forensics: dropper replaces 4.2.1 with 4.2.0 stub to hide evidence
          results.findings.push({
            type: 'VERSION_SPOOF_DETECTED',
            severity: 'HIGH',
            component: 'plain-crypto-js',
            version,
            path: nodeModulesPath,
            message: `Found plain-crypto-js@${version} — likely post-infection version spoof (dropper replaces 4.2.1 with 4.2.0 stub)`,
            remediation: 'Remove node_modules and reinstall: rm -rf node_modules package-lock.json && npm install'
          });
          results.summary.compromised = true;
          results.summary.severity = 'CRITICAL';
          results.summary.confidence += 30;
        }
      }
    } catch (err) {
      console.error('Error checking plain-crypto-js:', err.message);
    }
  }
}

/**
 * Check for RAT persistence files on Windows
 */
function checkPersistenceIndicators() {
  CONFIG.PERSISTENCE_PATHS.forEach(filePath => {
    if (fs.existsSync(filePath)) {
      results.findings.push({
        type: 'PERSISTENCE_FILE',
        severity: 'CRITICAL',
        path: filePath,
        message: `Found suspected RAT persistence file: ${filePath}`,
        remediation: `Delete file and scan with antivirus: del "${filePath}"`
      });
      results.summary.compromised = true;
      results.summary.severity = 'CRITICAL';
      results.summary.confidence += 35;
    }
  });
}

/**
 * Check network connections for C2 communication
 */
function checkNetworkIndicators() {
  try {
    const command = getNetstatCommand();
    const output = execSync(command, { encoding: 'utf8' });

    const connections = parseNetworkConnections(output);

    connections.forEach(conn => {
      if (CONFIG.C2_IPS.includes(conn.remoteIp) || CONFIG.C2_DOMAINS.some(d => conn.remoteHost?.includes(d))) {
        results.findings.push({
          type: 'C2_CONNECTION',
          severity: 'CRITICAL',
          connection: conn,
          message: `Found active connection to suspected C2: ${conn.remoteIp}:${conn.remotePort}`,
          remediation: 'Isolate machine immediately and investigate'
        });
        results.summary.compromised = true;
        results.summary.severity = 'CRITICAL';
        results.summary.confidence += 50;
      }
    });
  } catch (err) {
    results.findings.push({
      type: 'NETWORK_CHECK_FAILED',
      severity: 'LOW',
      message: `Could not check network connections: ${err.message}`,
      remediation: 'Requires elevated privileges; run with administrator/sudo'
    });
  }
}

/**
 * Get netstat command for Windows
 */
function getNetstatCommand() {
  return 'netstat -ano';
}

/**
 * Parse netstat output into connection objects
 */
function parseNetworkConnections(output) {
  const connections = [];
  const lines = output.split('\n').slice(4); // Skip headers

  lines.forEach(line => {
    const parts = line.trim().split(/\s+/);
    if (parts.length < 4) return;

    const [_, localAddr, remoteAddr, state] = parts;

    if (remoteAddr && state === 'ESTABLISHED') {
      const [remoteIp, remotePort] = remoteAddr.split(':');
      connections.push({ remoteIp, remotePort, remoteHost: remoteAddr });
    }
  });

  return connections;
}

/**
 * Determine overall risk level
 */
function calculateRisk() {
  if (results.findings.some(f => f.severity === 'CRITICAL')) {
    results.summary.confidence = Math.min(100, results.summary.confidence);
    results.summary.severity = 'CRITICAL';
  } else if (results.findings.some(f => f.severity === 'MEDIUM')) {
    results.summary.severity = 'MEDIUM';
  }
}

/**
 * Format and output results
 */
function outputResults(asJson = false) {
  calculateRisk();

  if (asJson) {
    console.log(JSON.stringify(results, null, 2));
  } else {
    console.log('\n' + '='.repeat(70));
    console.log('AXIOS COMPROMISE DETECTION REPORT');
    console.log('='.repeat(70));
    console.log(`Timestamp: ${results.timestamp}`);
    console.log(`Platform: ${results.platform}`);
    console.log(`Compromised: ${results.summary.compromised ? 'YES' : 'NO'}`);
    if (results.summary.compromised) {
      console.log(`Severity: ${results.summary.severity}`);
      console.log(`Confidence: ${results.summary.confidence}%`);
    }
    console.log('='.repeat(70));

    if (results.findings.length === 0) {
      console.log('\n✓ No indicators of compromise detected.');
    } else {
      console.log(`\n⚠ Found ${results.findings.length} indicator(s) of compromise:\n`);

      results.findings.forEach((finding, idx) => {
        console.log(`[${idx + 1}] ${finding.type} [${finding.severity}]`);
        console.log(`    Message: ${finding.message}`);
        if (finding.path) console.log(`    Path: ${finding.path}`);
        if (finding.component) console.log(`    Component: ${finding.component}`);
        if (finding.version) console.log(`    Version: ${finding.version}`);
        console.log(`    Action: ${finding.remediation}`);
        console.log();
      });
    }

    console.log('='.repeat(70));
  }
}

/**
 * Main execution
 */
function main() {
  const args = process.argv.slice(2);
  let scanDir = process.cwd();
  let checkNetwork = false;
  let jsonOutput = false;

  // Parse arguments
  for (let i = 0; i < args.length; i++) {
    if (args[i] === '--scan-dir' && args[i + 1]) {
      scanDir = args[++i];
    } else if (args[i] === '--check-network') {
      checkNetwork = true;
    } else if (args[i] === '--json') {
      jsonOutput = true;
    }
  }

  console.log('Scanning for axios compromise indicators...');

  // Run checks
  checkAxiosVersion(scanDir);
  checkPhantomDependency(scanDir);
  checkMaliciousDependency(scanDir);
  checkPersistenceIndicators();

  if (checkNetwork) {
    console.log('Checking network connections (may require elevated privileges)...');
    checkNetworkIndicators();
  }

  // Output results
  outputResults(jsonOutput);

  // Exit with status
  process.exit(results.summary.compromised ? 1 : 0);
}

// Run if executed directly
if (require.main === module) {
  main();
}

module.exports = { results, CONFIG };
