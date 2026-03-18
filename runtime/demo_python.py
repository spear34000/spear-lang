def render(payload):
    name = "world"
    if isinstance(payload, dict):
        name = payload.get("name", name)
    return f"python:{name}"
