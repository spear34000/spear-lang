import importlib
import json
import sys
from pathlib import Path


def main() -> int:
    _, target, fn, req_path, res_path = sys.argv
    try:
        payload = Path(req_path).read_text(encoding="utf-8")
        module = importlib.import_module(target)
        candidate = getattr(module, fn, None)
        if not callable(candidate):
            raise RuntimeError(f"python bridge: function '{fn}' was not found in '{target}'")
        arg = json.loads(payload) if payload else None
        result = candidate(arg)
        if isinstance(result, str):
            text = result
        else:
            text = json.dumps(result)
        Path(res_path).write_text(text, encoding="utf-8")
        return 0
    except Exception as exc:  # noqa: BLE001
        Path(res_path).write_text(str(exc), encoding="utf-8")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
