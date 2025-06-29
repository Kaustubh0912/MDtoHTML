# 📄 Markdown to HTML Converter

A lightweight, single-file C++ command-line tool that converts Markdown files into beautifully styled HTML documents. No external dependencies required for compilation.

[![Built with C++17](https://img.shields.io/badge/Built%20With-C%2B%2B17-blue?style=for-the-badge&logo=cplusplus)](https://isocpp.org/)
[![MIT License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)
[![Single File](https://img.shields.io/badge/Design-Single%20File-orange?style=for-the-badge)](main.cpp)

## ✨ Features

### Core Markdown Support
- 📝 **Headings**: H1-H6 with automatic ID generation
- 📄 **Text Formatting**: Paragraphs and line breaks
- 🎨 **Inline Styles**: **Bold**, *italic*, ~~strikethrough~~, `inline code`
- 📋 **Lists**: Unordered, ordered, and nested lists with proper indentation
- ✅ **Task Lists**: GitHub-style checkboxes `- [x]` and `- [ ]`
- 📊 **Tables**: Full table support with headers and auto-detection
- 🔗 **Links & Media**: `[text](url)` links, `![alt](src)` images, auto-linked URLs
- 💬 **Blockquotes**: `>` quoted text with styling
- 📏 **Horizontal Rules**: `---`, `***`, `___` separators

### Advanced Features
- 🔢 **Footnotes**: `[^1]` references with automatic backlinks
- 🧮 **LaTeX Math**: Inline `$...$` and block `$$...$$` equations via MathJax
- 😊 **Emoji Support**: Convert `:emoji:` codes (20+ built-in emojis)
- 🎯 **Table of Contents**: Auto-generated from headings with anchor links
- 🌙 **Dark/Light Mode**: Runtime theme toggle with persistence
- 🎨 **Syntax Highlighting**: Fenced code blocks with Highlight.js
- 📱 **Responsive Design**: Mobile-friendly CSS styling

## 🚀 Quick Start

### Compilation

```bash
# Clone or download main.cpp
g++ -std=c++17 -O2 -o md2html main.cpp
```

**Requirements**: Any C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Usage

```bash
# Convert specific files
./md2html input.md output.html

# Use defaults (input.md → output.html)
./md2html

# Examples
./md2html README.md docs.html
./md2html sample.md
```

## 📖 Supported Markdown Examples

### Basic Formatting
```markdown
# Main Title
## Subtitle

This is **bold text** and *italic text*.
Here's some `inline code` and ~~strikethrough~~.

> This is a blockquote
> spanning multiple lines.
```

### Lists and Tasks
```markdown
- Unordered list item
  - Nested item
    - Deep nested item
- Another item

1. Ordered list
2. Second item
3. Third item

- [x] Completed task
- [ ] Pending task
- [x] Another completed task
```

### Tables
```markdown
| Feature | Status | Notes |
|---------|--------|-------|
| Parsing | ✅ Done | Core functionality |
| Styling | ✅ Done | CSS included |
| Testing | 🔄 WIP | In progress |
```

### Code Blocks
```markdown
```cpp
#include <iostream>
int main() {
    std::cout << "Hello World!" << std::endl;
    return 0;
}
```

### Math and Emojis
```markdown
Inline math: $E = mc^2$

Block math:
$$
\sum_{i=1}^{n} i = \frac{n(n+1)}{2}
$$

Emojis: :rocket: :fire: :thumbsup: :smile:
```

### Footnotes
```markdown
This has a footnote[^1].

[^1]: This is the footnote content.
```

## ⚙️ Technical Details

### Built-in Features
- **Zero Dependencies**: Single C++ file with standard library only
- **Web CDN Integration**: MathJax and Highlight.js loaded from CDN
- **20+ Emoji Mappings**: Common emoji shortcodes supported
- **Regex-based Parsing**: Robust pattern matching for markdown elements
- **HTML Escaping**: Automatic escaping for security
- **Memory Efficient**: Stream-based processing

### Generated HTML Features
- **Responsive CSS**: Works on all screen sizes
- **Dark/Light Toggle**: Persistent theme switching
- **Syntax Highlighting**: 180+ languages supported via Highlight.js
- **MathJax Integration**: Beautiful math rendering
- **Table of Contents**: Auto-generated from headings
- **Modern Styling**: Clean, GitHub-inspired design

### Supported Emoji Codes
```
:smile: 😊    :heart: ❤️     :thumbsup: 👍   :fire: 🔥
:star: ⭐     :rocket: 🚀    :tada: 🎉      :eyes: 👀
:laugh: 😂    :cry: 😢      :angry: 😠     :cool: 😎
:wink: 😉     :thinking: 🤔  :check: ✅     :x: ❌
:warning: ⚠️  :info: ℹ️      :bulb: 💡      :thumbsdown: 👎
```

## 🎨 Output Features

The generated HTML includes:
- **Professional Styling**: Modern, clean design with proper typography
- **Interactive Elements**: Hover effects, smooth transitions
- **Theme Toggle**: Dark/light mode button (top-right corner)
- **Responsive Layout**: Adapts to different screen sizes
- **Print Friendly**: Optimized for printing
- **Accessible**: Proper semantic HTML structure



# Convert
```
./md2html test.md test.html
```
# Open in browser
```
open test.html  # macOS

xdg-open test.html  # Linux

start test.html  # Windows
```

## 🔧 Customization

Since this is a single-file implementation, you can easily modify:

### Adding More Emojis
Edit the `initializeEmojiMap()` function in `main.cpp`:
```cpp
emojiMap[":custom:"] = "🎯";
```

### Changing Styles
Modify the CSS in the `generateHTML()` function:
```cpp
// Find this section and modify colors/fonts
":root {\n"
"  --bg-color: #ffffff;\n"
"  --text-color: #333333;\n"
```

### Adding Features
The `MarkdownConverter` class can be extended with new methods for additional markdown features.

## 📋 Limitations

- **No Plugin System**: All features are built-in
- **Single File Input**: Processes one file at a time
- **Basic Nesting**: Limited support for complex nested structures
- **No Live Preview**: Static conversion only
- **Fixed Emoji Set**: Only 20+ predefined emoji codes

## 🚧 Potential Improvements

- [ ] Command-line options for disabling features
- [ ] Custom CSS file support
- [ ] Batch file processing
- [ ] More comprehensive emoji database
- [ ] Better error reporting
- [ ] Configuration file support

## 📄 License

This project is licensed under the [MIT License](LICENSE).


---

<div align="center">

**Simple, Fast, Effective** - A single C++ file that does it all!

[Download main.cpp](main.cpp) · [Report Issues](https://github.com/Kaustubh0912/MDtoHTML/issues) 

</div>
