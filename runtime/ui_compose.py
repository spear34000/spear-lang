from __future__ import annotations

from ui_common import (
    compose_action_expression,
    compose_string,
    node_body,
    node_items,
    node_kind,
    screen_meta,
    state_items,
    text_value,
)


def _state_lines(spec):
    lines = []
    for item in state_items(spec):
        name = text_value(item.get("name", "state")) or "state"
        value = item.get("value", "")
        state_type = text_value(item.get("state_type", "text"))
        if state_type == "num":
            lines.append(f"    var {name} by remember {{ mutableIntStateOf({int(value)}) }}\n")
        else:
            lines.append(f"    var {name} by remember {{ mutableStateOf({compose_string(value)}) }}\n")
    return "".join(lines)


def _render_children(items, indent):
    return "".join(_render_node(item, indent) for item in items)


def _action_block(action, indent):
    action_expr = compose_action_expression(action)
    pad = " " * indent
    return f"{pad}val actionMeta = {action_expr}\n"


def _render_node(node, indent):
    pad = " " * indent
    kind = node_kind(node)
    if kind == "column":
        body = _render_children(node_items(node), indent + 4)
        return f"{pad}Column(verticalArrangement = Arrangement.spacedBy(16.dp)) {{\n{body}{pad}}}\n"
    if kind == "row":
        body = _render_children(node_items(node), indent + 4)
        return f"{pad}Row(horizontalArrangement = Arrangement.spacedBy(16.dp)) {{\n{body}{pad}}}\n"
    if kind == "form":
        body = _render_children(node_items(node), indent + 4)
        return f"{pad}Column(verticalArrangement = Arrangement.spacedBy(16.dp)) {{\n{body}{pad}}}\n"
    if kind == "nav":
        body = _render_children(node_items(node), indent + 4)
        return f"{pad}Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {{\n{body}{pad}}}\n"
    if kind == "toolbar":
        title = compose_string(node.get("title", ""))
        left = _render_children(node_items({"items": node.get("left", [])}), indent + 8)
        right = _render_children(node_items({"items": node.get("right", [])}), indent + 8)
        return (
            f"{pad}Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {{\n"
            f"{pad}    Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {{\n{left}{pad}    }}\n"
            f"{pad}    if ({title} != \"\") {{ Text(text = {title}, style = MaterialTheme.typography.titleMedium) }}\n"
            f"{pad}    Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {{\n{right}{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "tabs":
        body = _render_children(node_items(node), indent + 4)
        return f"{pad}Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {{\n{body}{pad}}}\n"
    if kind == "list":
        body = _render_children(node_items(node), indent + 4)
        return f"{pad}Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {{\n{body}{pad}}}\n"
    if kind == "item":
        title = text_value(node.get("title", ""))
        header = f"{pad}        Text(text = {compose_string(title)}, style = MaterialTheme.typography.titleSmall)\n" if title else ""
        body = _render_node(node_body(node), indent + 8)
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {{\n"
            f"{header}{body}"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "title":
        return f"{pad}Text(text = {compose_string(node.get('text', ''))}, style = MaterialTheme.typography.headlineLarge)\n"
    if kind == "text":
        return f"{pad}Text(text = {compose_string(node.get('text', ''))}, style = MaterialTheme.typography.bodyLarge)\n"
    if kind == "notice":
        title = compose_string(node.get("title", "Notice"))
        text = compose_string(node.get("text", ""))
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {{\n"
            f"{pad}        Text(text = {title}, style = MaterialTheme.typography.titleMedium)\n"
            f"{pad}        Text(text = {text}, style = MaterialTheme.typography.bodyMedium)\n"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "loading":
        return f"{pad}Text(text = {compose_string(node.get('text', 'Loading'))}, style = MaterialTheme.typography.bodyMedium)\n"
    if kind == "empty":
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {{\n"
            f"{pad}        Text(text = {compose_string(node.get('title', 'Empty'))}, style = MaterialTheme.typography.titleMedium)\n"
            f"{pad}        Text(text = {compose_string(node.get('text', ''))}, style = MaterialTheme.typography.bodyMedium)\n"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "error":
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {{\n"
            f"{pad}        Text(text = {compose_string(node.get('title', 'Error'))}, style = MaterialTheme.typography.titleMedium)\n"
            f"{pad}        Text(text = {compose_string(node.get('text', ''))}, style = MaterialTheme.typography.bodyMedium)\n"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "button":
        label = compose_string(node.get("label", "Action"))
        return (
            f"{pad}Button(onClick = {{\n"
            f"{_action_block(node.get('action', {}), indent + 4)}"
            f"{pad}}}) {{ Text(text = {label}) }}\n"
        )
    if kind == "link":
        label = compose_string(node.get("label", "Open"))
        return (
            f"{pad}TextButton(onClick = {{\n"
            f"{_action_block(node.get('action', {'type': 'go', 'route': node.get('route', '')}), indent + 4)}"
            f"{pad}}}) {{ Text(text = {label}) }}\n"
        )
    if kind == "input":
        state = text_value(node.get("state", "value")) or "value"
        label = compose_string(node.get("label", "Field"))
        placeholder = compose_string(node.get("placeholder", ""))
        hint = text_value(node.get("hint", ""))
        error_text = text_value(node.get("error_text", ""))
        lines = [
            f"{pad}Column(verticalArrangement = Arrangement.spacedBy(6.dp)) {{\n",
            f"{pad}    OutlinedTextField(\n",
            f"{pad}        value = {state},\n",
            f"{pad}        onValueChange = {{ {state} = it }},\n",
            f"{pad}        label = {{ Text(text = {label}) }},\n",
            f"{pad}        placeholder = {{ Text(text = {placeholder}) }},\n",
            f"{pad}        modifier = Modifier.fillMaxWidth(),\n",
            f"{pad}        enabled = {'false' if bool(node.get('disabled', False)) else 'true'},\n",
            f"{pad}        isError = {'true' if error_text else 'false'}\n",
            f"{pad}    )\n",
        ]
        if hint:
            lines.append(f"{pad}    Text(text = {compose_string(hint)}, style = MaterialTheme.typography.bodySmall)\n")
        if error_text:
            lines.append(f"{pad}    Text(text = {compose_string(error_text)}, style = MaterialTheme.typography.bodySmall)\n")
        lines.append(f"{pad}}}\n")
        return "".join(lines)
    if kind == "stat":
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {{\n"
            f"{pad}        Text(text = {compose_string(node.get('label', 'Metric'))}, style = MaterialTheme.typography.labelMedium)\n"
            f"{pad}        Text(text = {compose_string(node.get('value', '0'))}, style = MaterialTheme.typography.headlineSmall)\n"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "card":
        title = text_value(node.get("title", ""))
        heading = f"{pad}        Text(text = {compose_string(title)}, style = MaterialTheme.typography.titleLarge)\n" if title else ""
        body = _render_node(node_body(node), indent + 8)
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(20.dp), verticalArrangement = Arrangement.spacedBy(14.dp)) {{\n"
            f"{heading}{body}"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "section":
        title = text_value(node.get("title", ""))
        heading = f"{pad}    Text(text = {compose_string(title)}, style = MaterialTheme.typography.titleLarge)\n" if title else ""
        body = _render_node(node_body(node), indent + 4)
        return f"{pad}Column(verticalArrangement = Arrangement.spacedBy(14.dp)) {{\n{heading}{body}{pad}}}\n"
    if kind == "dialog":
        body = _render_node(node_body(node), indent + 8)
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(20.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {{\n"
            f"{pad}        Text(text = {compose_string(node.get('title', 'Dialog'))}, style = MaterialTheme.typography.titleLarge)\n"
            f"{body}"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "sheet":
        body = _render_node(node_body(node), indent + 8)
        return (
            f"{pad}Card(modifier = Modifier.fillMaxWidth()) {{\n"
            f"{pad}    Column(modifier = Modifier.padding(20.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {{\n"
            f"{pad}        Text(text = {compose_string(node.get('title', 'Sheet'))}, style = MaterialTheme.typography.titleLarge)\n"
            f"{body}"
            f"{pad}    }}\n"
            f"{pad}}}\n"
        )
    if kind == "divider":
        return f"{pad}HorizontalDivider()\n"
    return ""


def _compose_header(spec):
    route = text_value(spec.get("route", ""))
    title = text_value(spec.get("title", ""))
    lines = []
    if route or title:
        meta_parts = []
        if route:
            meta_parts.append(f'"route" to {compose_string(route)}')
        if title:
            meta_parts.append(f'"title" to {compose_string(title)}')
        lines.append(f"    val screenMeta = mapOf({', '.join(meta_parts)})\n")
    return "".join(lines)


def _compose_function(screen_name, spec):
    states = _state_lines(spec)
    header = _compose_header(spec)
    body = _render_node(node_body(spec), 8)
    return (
        f"@Composable\nfun {screen_name}() {{\n"
        f"{states}"
        f"{header}"
        "    Surface(modifier = Modifier.fillMaxSize(), color = Color(0xFFF5F3EE)) {\n"
        "        Column(\n"
        "            modifier = Modifier.fillMaxSize().padding(24.dp),\n"
        "            verticalArrangement = Arrangement.spacedBy(16.dp)\n"
        "        ) {\n"
        f"{body}"
        "        }\n"
        "    }\n"
        "}\n"
    )


def ui_android_render(payload):
    meta = screen_meta(payload, "MainScreen")
    screen_name = meta["screen_name"] or "MainScreen"
    spec = meta["screen"]
    return (
        "import androidx.compose.foundation.layout.*\n"
        "import androidx.compose.material3.*\n"
        "import androidx.compose.runtime.*\n"
        "import androidx.compose.ui.Modifier\n"
        "import androidx.compose.ui.graphics.Color\n"
        "import androidx.compose.ui.unit.dp\n\n"
        "private fun spearAction(vararg pairs: Pair<String, String>): Map<String, String> = mapOf(*pairs)\n\n"
        f"{_compose_function(screen_name, spec)}"
    )


def ui_desktop_render(payload):
    meta = screen_meta(payload, "DesktopScreen")
    screen_name = meta["screen_name"] or "DesktopScreen"
    spec = meta["screen"]
    return (
        "import androidx.compose.foundation.layout.*\n"
        "import androidx.compose.material3.*\n"
        "import androidx.compose.runtime.*\n"
        "import androidx.compose.ui.Modifier\n"
        "import androidx.compose.ui.graphics.Color\n"
        "import androidx.compose.ui.unit.dp\n"
        "import androidx.compose.ui.window.Window\n"
        "import androidx.compose.ui.window.application\n\n"
        "private fun spearAction(vararg pairs: Pair<String, String>): Map<String, String> = mapOf(*pairs)\n\n"
        f"{_compose_function(screen_name, spec)}\n"
        "fun main() = application {\n"
        f"    Window(onCloseRequest = ::exitApplication, title = {compose_string(meta['title'] or screen_name)}) {{\n"
        f"        {screen_name}()\n"
        "    }\n"
        "}\n"
    )
