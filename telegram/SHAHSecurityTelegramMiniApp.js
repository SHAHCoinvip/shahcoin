// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Hamid Rad / SHAHCOIN Core

(function () {
  const BLOCKLIST = {
    domains: ['shahco1n.com', 'shahc0in.com', 'shahcoin-airdrop.com', 'free-shah.vip', 'claim-shah.vip'],
    patterns: [/airdrop/i, /free.*mint/i, /claim.*shah/i, /connect.*wallet.*now/i, /verify.*seed/i],
    allowlist: ['shah.vip', 'app.shah.vip']
  };

  function checkUrlSafety(url) {
    try {
      const u = new URL(url);
      const host = u.hostname.toLowerCase();
      if (BLOCKLIST.allowlist.some(d => host.endsWith(d))) {
        return { status: 'safe', reason: 'allowlist', host };
      }
      if (BLOCKLIST.domains.includes(host)) {
        return { status: 'phishing', reason: 'blocklist', host };
      }
      const full = url.toLowerCase();
      if (BLOCKLIST.patterns.some(re => re.test(full))) {
        return { status: 'phishing', reason: 'pattern', host };
      }
      return { status: 'unknown', reason: 'no_match', host };
    } catch (e) {
      return { status: 'invalid', reason: 'parse_error', host: '' };
    }
  }

  function showAlert(message) {
    if (window.Telegram && window.Telegram.WebApp) {
      window.Telegram.WebApp.showAlert(message);
    } else {
      alert(message);
    }
  }

  function warnBeforeOpen(url) {
    const result = checkUrlSafety(url);
    if (result.status === 'phishing') {
      showAlert('⚠️ Dangerous URL detected: ' + result.host + '\nOpening blocked.');
      return false;
    }
    if (result.status === 'unknown') {
      return confirm('This link is not verified. Proceed?\n' + url);
    }
    return true;
  }

  // Expose minimal API
  window.SHAHSecurityTelegram = {
    checkUrlSafety,
    warnBeforeOpen
  };
})();
