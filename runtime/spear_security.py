from __future__ import annotations

import base64
import hashlib
import hmac
import json
import secrets
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path


def _text(value):
    if value is None:
        return ""
    if isinstance(value, str):
        return value
    return json.dumps(value, ensure_ascii=False)


class _NoRedirectHandler(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        raise urllib.error.HTTPError(newurl, code, msg, headers, fp)


def secure_token(payload):
    size = int(payload.get("bytes", 32))
    if size < 16:
        size = 16
    if size > 64:
        size = 64
    return secrets.token_urlsafe(size)


def secure_compare(payload):
    left = _text(payload.get("left", ""))
    right = _text(payload.get("right", ""))
    return 1 if hmac.compare_digest(left, right) else 0


def pbkdf2_sha256(payload):
    password = payload.get("password", "").encode("utf-8")
    salt = payload.get("salt", "").encode("utf-8")
    rounds = int(payload.get("rounds", 120000))
    if rounds < 100000:
        rounds = 100000
    if rounds > 1000000:
        rounds = 1000000
    return hashlib.pbkdf2_hmac("sha256", password, salt, rounds).hex()


def pbkdf2_verify(payload):
    expected = _text(payload.get("expected", ""))
    actual = pbkdf2_sha256(payload)
    return 1 if hmac.compare_digest(actual, expected) else 0


def safe_http_request(payload):
    method = payload.get("method", "GET").upper()
    url = payload.get("url", "")
    body = payload.get("body", "")
    timeout = float(payload.get("timeout", 10))
    max_bytes = int(payload.get("max_bytes", 262144))
    content_type = payload.get("content_type", "")
    timeout = min(max(timeout, 1.0), 30.0)
    max_bytes = min(max(max_bytes, 1024), 1048576)
    parsed = urllib.parse.urlparse(url)
    if parsed.scheme != "https":
        return {"ok": False, "error": "https is required"}
    if not parsed.netloc:
        return {"ok": False, "error": "missing https host"}
    headers = {"User-Agent": "Spear/0.1"}
    if content_type:
        headers["Content-Type"] = content_type
    data = body.encode("utf-8") if body else None
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    opener = urllib.request.build_opener(_NoRedirectHandler)
    try:
        with opener.open(req, timeout=timeout) as response:
            data = response.read(max_bytes + 1)
            if len(data) > max_bytes:
                return {"ok": False, "error": "response body too large"}
            return {
                "ok": True,
                "status": response.status,
                "body": data.decode("utf-8", errors="replace"),
                "headers": dict(response.headers.items()),
                "url": url,
            }
    except urllib.error.HTTPError as exc:
        body_bytes = exc.read(max_bytes + 1)
        if len(body_bytes) > max_bytes:
            return {"ok": False, "error": "response body too large"}
        return {
            "ok": False,
            "status": exc.code,
            "body": body_bytes.decode("utf-8", errors="replace"),
            "error": f"http status {exc.code}",
            "headers": dict(exc.headers.items()),
            "url": url,
        }
    except Exception as exc:  # noqa: BLE001
        return {"ok": False, "error": str(exc), "url": url}


def jwt_verify_hs256(payload):
    token = payload.get("token", "")
    key = payload.get("key", "").encode("utf-8")
    leeway = int(payload.get("leeway", 0))
    parts = token.split(".")
    if len(parts) != 3:
        return {"ok": False, "error": "invalid token format"}

    def decode_part(text):
        pad = "=" * (-len(text) % 4)
        raw = base64.urlsafe_b64decode((text + pad).encode("ascii"))
        return json.loads(raw.decode("utf-8"))

    try:
        header = decode_part(parts[0])
        claims = decode_part(parts[1])
    except Exception:  # noqa: BLE001
        return {"ok": False, "error": "invalid token payload"}

    if not isinstance(header, dict) or header.get("alg") != "HS256":
        return {"ok": False, "error": "unsupported jwt algorithm"}
    signing = f"{parts[0]}.{parts[1]}".encode("ascii")
    expected = base64.urlsafe_b64encode(hmac.new(key, signing, hashlib.sha256).digest()).rstrip(b"=").decode("ascii")
    if not hmac.compare_digest(expected, parts[2]):
        return {"ok": False, "error": "invalid jwt signature"}
    now = int(time.time())
    if isinstance(claims, dict):
        exp = claims.get("exp")
        nbf = claims.get("nbf")
        if isinstance(exp, (int, float)) and now > int(exp) + leeway:
            return {"ok": False, "error": "jwt has expired"}
        if isinstance(nbf, (int, float)) and now + leeway < int(nbf):
            return {"ok": False, "error": "jwt is not active yet"}
    return {"ok": True, "claims": claims}


def _resolve_under_root(root_text, relative_text):
    root = Path(root_text).resolve()
    target = (root / relative_text).resolve()
    try:
        target.relative_to(root)
    except ValueError as exc:
        raise RuntimeError("path escapes root") from exc
    return target


def fs_safe_read(payload):
    try:
        path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    except RuntimeError as exc:
        return {"ok": False, "error": str(exc)}
    if not path.exists() or not path.is_file():
        return {"ok": False, "error": "file was not found"}
    return {"ok": True, "text": path.read_text(encoding="utf-8"), "path": str(path)}


def fs_safe_write(payload):
    try:
        path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    except RuntimeError as exc:
        return {"ok": False, "error": str(exc)}
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(payload.get("content", ""), encoding="utf-8")
    return {"ok": True, "path": str(path)}


def fs_safe_list(payload):
    try:
        path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    except RuntimeError as exc:
        return {"ok": False, "error": str(exc)}
    if not path.exists() or not path.is_dir():
        return {"ok": False, "error": "directory was not found"}
    return {"ok": True, "items": [item.name for item in path.iterdir()], "path": str(path)}


def fs_safe_mkdir(payload):
    try:
        path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    except RuntimeError as exc:
        return {"ok": False, "error": str(exc)}
    path.mkdir(parents=True, exist_ok=True)
    return {"ok": True, "path": str(path)}


def fs_safe_remove(payload):
    try:
        path = _resolve_under_root(payload.get("root", "."), payload.get("path", ""))
    except RuntimeError as exc:
        return {"ok": False, "error": str(exc)}
    if path.is_dir():
        if any(path.iterdir()):
            return {"ok": False, "error": "refusing to remove non-empty directory"}
        path.rmdir()
        return {"ok": True, "path": str(path)}
    if path.exists():
        path.unlink()
        return {"ok": True, "path": str(path)}
    return {"ok": False, "error": "path was not found"}
