import "text.sp";

package std;
module mobile;

function text ui2(text a, text b) {
    return join(a, b);
}

function text ui3(text a, text b, text c) {
    return join(join(a, b), c);
}

function text ui4(text a, text b, text c, text d) {
    return join(join(join(a, b), c), d);
}

function text android_title(text value) {
    return join4("                Text(text = ", quote(value), ", style = MaterialTheme.typography.headlineLarge)\n", "");
}

function text android_body(text value) {
    return join4("                Text(text = ", quote(value), ", style = MaterialTheme.typography.bodyLarge)\n", "");
}

function text android_button(text label) {
    return join4("                Button(onClick = { }) { Text(text = ", quote(label), ") }\n", "");
}

function text android_card(text title, text body) {
    sharp {
        text start = "                Card(modifier = Modifier.fillMaxWidth()) {\n                    Column(modifier = Modifier.padding(20.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {\n";
        text head = join4("                        Text(text = ", quote(title), ", style = MaterialTheme.typography.titleLarge)\n", "");
        text inner = join(join(start, head), body);
        return join(inner, "                    }\n                }\n");
    }
}

function text android_stat(text label, text value) {
    sharp {
        text top = join4("                        Text(text = ", quote(label), ", style = MaterialTheme.typography.labelMedium)\n", "");
        text bottom = join4("                        Text(text = ", quote(value), ", style = MaterialTheme.typography.headlineSmall)\n", "");
        return join(top, bottom);
    }
}

function text android_column(text body) {
    sharp {
        text start = "        Column(\n            modifier = Modifier.fillMaxSize().padding(24.dp),\n            verticalArrangement = Arrangement.spacedBy(16.dp)\n        ) {\n";
        text inner = join(start, body);
        return join(inner, "        }\n");
    }
}

function text android_screen(text screen_name, text bg_color, text body) {
    sharp {
        text top = "import androidx.compose.foundation.layout.*\nimport androidx.compose.material3.*\nimport androidx.compose.runtime.Composable\nimport androidx.compose.ui.Modifier\nimport androidx.compose.ui.graphics.Color\nimport androidx.compose.ui.unit.dp\n\n";
        text header = join4("@Composable\nfun ", screen_name, "() {\n", "");
        text surface = join4("    Surface(modifier = Modifier.fillMaxSize(), color = Color(", bg_color, ")) {\n", "");
        text full = join(join(join(top, header), surface), body);
        return join(full, "    }\n}\n");
    }
}
