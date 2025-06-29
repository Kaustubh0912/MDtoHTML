#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

using namespace std;

struct TocEntry
{
    int level;
    string id;
    string text;
};

class MarkdownConverter
{
private:
    bool inList = false;
    bool inOrderedList = false;
    bool inCodeBlock = false;
    bool inTable = false;
    int listDepth = 0;
    vector<TocEntry> tocEntries;
    map<string, string> footnotes;
    map<string, string> emojiMap;

    void initializeEmojiMap()
    {
        emojiMap[":smile:"] = "😊";
        emojiMap[":heart:"] = "❤️";
        emojiMap[":thumbsup:"] = "👍";
        emojiMap[":thumbsdown:"] = "👎";
        emojiMap[":fire:"] = "🔥";
        emojiMap[":star:"] = "⭐";
        emojiMap[":rocket:"] = "🚀";
        emojiMap[":tada:"] = "🎉";
        emojiMap[":eyes:"] = "👀";
        emojiMap[":laugh:"] = "😂";
        emojiMap[":cry:"] = "😢";
        emojiMap[":angry:"] = "😠";
        emojiMap[":cool:"] = "😎";
        emojiMap[":wink:"] = "😉";
        emojiMap[":thinking:"] = "🤔";
        emojiMap[":check:"] = "✅";
        emojiMap[":x:"] = "❌";
        emojiMap[":warning:"] = "⚠️";
        emojiMap[":info:"] = "ℹ️";
        emojiMap[":bulb:"] = "💡";
    }

    string trim(const string &str)
    {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == string::npos)
            return "";
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    string escapeHtml(const string &text)
    {
        string escaped = text;
        size_t pos = 0;
        while ((pos = escaped.find("&", pos)) != string::npos)
        {
            escaped.replace(pos, 1, "&amp;");
            pos += 5;
        }
        pos = 0;
        while ((pos = escaped.find("<", pos)) != string::npos)
        {
            escaped.replace(pos, 1, "&lt;");
            pos += 4;
        }
        pos = 0;
        while ((pos = escaped.find(">", pos)) != string::npos)
        {
            escaped.replace(pos, 1, "&gt;");
            pos += 4;
        }
        return escaped;
    }

    string processEmojis(string text)
    {
        for (const auto &emoji : emojiMap)
        {
            size_t pos = 0;
            while ((pos = text.find(emoji.first, pos)) != string::npos)
            {
                text.replace(pos, emoji.first.length(), emoji.second);
                pos += emoji.second.length();
            }
        }
        return text;
    }

    string processMath(string line)
    {
        // Block math: $$...$$
        line = regex_replace(line, regex("\\$\\$([^$]+)\\$\\$"), "<div class=\"math-block\">$1</div>");

        // Inline math: $...$
        line = regex_replace(line, regex("\\$([^$]+)\\$"), "<span class=\"math-inline\">$1</span>");

        return line;
    }

    string generateId(const string &text)
    {
        string id = text;
        transform(id.begin(), id.end(), id.begin(), ::tolower);

        // Replace spaces and special chars with hyphens
        for (char &c : id)
        {
            if (!isalnum(c))
                c = '-';
        }

        // Remove multiple consecutive hyphens
        id = regex_replace(id, regex("-+"), "-");

        // Remove leading/trailing hyphens
        if (!id.empty() && id.front() == '-')
            id.erase(0, 1);
        if (!id.empty() && id.back() == '-')
            id.pop_back();

        return id;
    }

    string processTaskList(const string &line)
    {
        if (line.length() < 6)
            return line;

        if (line.substr(0, 6) == "- [x] " || line.substr(0, 6) == "- [X] ")
        {
            string content = trim(line.substr(6));
            return "<li class=\"task-item\"><input type=\"checkbox\" checked disabled> " + processInlineFormatting(content) + "</li>";
        }
        else if (line.substr(0, 6) == "- [ ] ")
        {
            string content = trim(line.substr(6));
            return "<li class=\"task-item\"><input type=\"checkbox\" disabled> " + processInlineFormatting(content) + "</li>";
        }

        return line;
    }

    vector<string> parseTableRow(const string &line)
    {
        vector<string> cells;
        string trimmed = trim(line);

        // Remove leading and trailing pipes if they exist
        if (!trimmed.empty() && trimmed.front() == '|')
        {
            trimmed = trimmed.substr(1);
        }
        if (!trimmed.empty() && trimmed.back() == '|')
        {
            trimmed = trimmed.substr(0, trimmed.length() - 1);
        }

        stringstream ss(trimmed);
        string cell;

        while (getline(ss, cell, '|'))
        {
            cells.push_back(trim(cell));
        }

        return cells;
    }

    bool isTableSeparator(const string &line)
    {
        vector<string> parts = parseTableRow(line);
        if (parts.empty())
            return false;

        for (const string &part : parts)
        {
            string trimmed = trim(part);
            if (trimmed.empty())
                return false;

            // Check if it contains only allowed characters: -, :, and spaces
            for (char c : trimmed)
            {
                if (c != '-' && c != ':' && c != ' ')
                {
                    return false;
                }
            }

            // Must contain at least one dash
            if (trimmed.find('-') == string::npos)
                return false;
        }

        return true;
    }

    int getIndentLevel(const string &line)
    {
        int level = 0;
        for (char c : line)
        {
            if (c == ' ')
                level++;
            else if (c == '\t')
                level += 4;
            else
                break;
        }
        return level / 2; // Assuming 2 spaces per indent level
    }

    string processFootnotes(string line)
    {
        try
        {
            // Process footnote references [^1]
            line = regex_replace(line, regex("\\[\\^([^\\]]+)\\]"), "<sup><a href=\"#fn$1\" id=\"fnref$1\">$1</a></sup>");
        }
        catch (const regex_error &e)
        {
            cerr << "Regex error in footnotes: " << e.what() << endl;
        }
        return line;
    }

    string processInlineFormatting(string line)
    {
        try
        {
            // Escape HTML first
            line = escapeHtml(line);

            // Process emojis
            line = processEmojis(line);

            // Process math
            line = processMath(line);

            // Process footnotes
            line = processFootnotes(line);

            // Code spans (must be processed before other formatting)
            line = regex_replace(line, regex("`([^`]+)`"), "<code>$1</code>");

            // Images: ![alt](src)
            line = regex_replace(line, regex("!\\[([^\\]]*)\\]\\(([^)]+)\\)"), "<img src=\"$2\" alt=\"$1\">");

            // Links: [text](url)
            line = regex_replace(line, regex("\\[([^\\]]+)\\]\\(([^)]+)\\)"), "<a href=\"$2\">$1</a>");

            // Auto-link raw URLs (http/https)
            line = regex_replace(line, regex(R"((https?://[^\s)]+))"), "<a href=\"$1\">$1</a>");

            // Bold
            line = regex_replace(line, regex("\\*\\*([^*]+)\\*\\*"), "<strong>$1</strong>");
            line = regex_replace(line, regex("__([^_]+)__"), "<strong>$1</strong>");

            // Italic
            line = regex_replace(line, regex("\\*([^*]+)\\*"), "<em>$1</em>");
            line = regex_replace(line, regex("_([^_]+)_"), "<em>$1</em>");

            // Strikethrough
            line = regex_replace(line, regex("~~([^~]+)~~"), "<del>$1</del>");
        }
        catch (const regex_error &e)
        {
            cerr << "Regex error in inline formatting: " << e.what() << endl;
        }

        return line;
    }

    string processHeading(const string &line)
    {
        int level = 0;
        for (char c : line)
        {
            if (c == '#')
                level++;
            else
                break;
        }

        if (level > 6)
            level = 6;

        string content = trim(line.substr(level));
        string id = generateId(content);

        // Add to TOC
        tocEntries.push_back({level, id, content});

        return "<h" + to_string(level) + " id=\"" + id + "\">" + processInlineFormatting(content) + "</h" + to_string(level) + ">";
    }

    void closeAllLists(string &html)
    {
        while (listDepth > 0)
        {
            if (inList)
            {
                html += "</ul>\n";
                inList = false;
            }
            if (inOrderedList)
            {
                html += "</ol>\n";
                inOrderedList = false;
            }
            listDepth--;
        }
    }

    bool isOrderedListItem(const string &line)
    {
        if (line.empty())
            return false;
        size_t i = 0;
        while (i < line.length() && isdigit(line[i]))
            i++;
        return i > 0 && i < line.length() && line[i] == '.' && i + 1 < line.length() && line[i + 1] == ' ';
    }

    string generateTOC()
    {
        if (tocEntries.empty())
            return "";

        string toc = "<div class=\"toc\">\n<h2>Table of Contents</h2>\n<ul>\n";

        for (const auto &entry : tocEntries)
        {
            string indent(entry.level - 1, ' ');
            toc += indent + "  <li><a href=\"#" + entry.id + "\">" + entry.text + "</a></li>\n";
        }

        toc += "</ul>\n</div>\n\n";
        return toc;
    }

    string generateFootnotes()
    {
        if (footnotes.empty())
            return "";

        string result = "\n<div class=\"footnotes\">\n<hr>\n<ol>\n";

        for (const auto &note : footnotes)
        {
            result += "<li id=\"fn" + note.first + "\">" + processInlineFormatting(note.second) +
                      " <a href=\"#fnref" + note.first + "\" class=\"footnote-backref\">↩</a></li>\n";
        }

        result += "</ol>\n</div>\n";
        return result;
    }

public:
    MarkdownConverter()
    {
        initializeEmojiMap();
    }

    string convertToHTML(const string &markdown)
    {
        string html;
        istringstream iss(markdown);
        string line;
        vector<string> lines;

        // First pass: collect all lines and extract footnotes
        while (iss.good() && getline(iss, line))
        {
            // Check for footnote definitions
            try
            {
                if (line.length() > 3 && line[0] == '[' && line[1] == '^')
                {
                    size_t closeBracket = line.find("]:");
                    if (closeBracket != string::npos && closeBracket > 2)
                    {
                        string footnoteId = line.substr(2, closeBracket - 2);
                        string footnoteText = line.substr(closeBracket + 2);
                        footnotes[footnoteId] = trim(footnoteText);
                        continue;
                    }
                }
            }
            catch (const exception &e)
            {
                cerr << "Error processing footnote definition: " << e.what() << endl;
            }
            lines.push_back(line);
        }

        // Second pass: process markdown
        for (size_t i = 0; i < lines.size(); i++)
        {
            line = trim(lines[i]);

            // Empty lines
            if (line.empty())
            {
                closeAllLists(html);
                if (inTable)
                {
                    html += "</table>\n";
                    inTable = false;
                }
                continue;
            }

            // Code blocks
            if (line.substr(0, 3) == "```")
            {
                closeAllLists(html);
                if (inTable)
                {
                    html += "</table>\n";
                    inTable = false;
                }
                if (!inCodeBlock)
                {
                    string lang = line.length() > 3 ? line.substr(3) : "";
                    html += "<pre><code" + (lang.empty() ? "" : " class=\"language-" + lang + "\"") + ">";
                    inCodeBlock = true;
                }
                else
                {
                    html += "</code></pre>\n";
                    inCodeBlock = false;
                }
                continue;
            }

            // Inside code block
            if (inCodeBlock)
            {
                html += escapeHtml(line) + "\n";
                continue;
            }

            // Table detection
            if (line.find('|') != string::npos && !inTable)
            {
                // Check if this might be a table
                if (i + 1 < lines.size() && isTableSeparator(lines[i + 1]))
                {
                    closeAllLists(html);
                    html += "<table>\n<thead>\n<tr>";

                    vector<string> headers = parseTableRow(line);
                    for (const string &header : headers)
                    {
                        html += "<th>" + processInlineFormatting(header) + "</th>";
                    }
                    html += "</tr>\n</thead>\n<tbody>\n";
                    inTable = true;
                    i++; // Skip separator line
                    continue;
                }
            }

            // Table rows
            if (inTable && line.find('|') != string::npos)
            {
                html += "<tr>";
                vector<string> cells = parseTableRow(line);
                for (const string &cell : cells)
                {
                    html += "<td>" + processInlineFormatting(cell) + "</td>";
                }
                html += "</tr>\n";
                continue;
            }
            else if (inTable)
            {
                html += "</tbody>\n</table>\n";
                inTable = false;
            }

            // Headings
            if (line[0] == '#')
            {
                closeAllLists(html);
                html += processHeading(line) + "\n";
            }
            // Horizontal rule
            else if (line == "---" || line == "***" || line == "___")
            {
                closeAllLists(html);
                html += "<hr>\n";
            }
            // Blockquotes
            else if (line[0] == '>')
            {
                closeAllLists(html);
                string content = trim(line.substr(1));
                html += "<blockquote><p>" + processInlineFormatting(content) + "</p></blockquote>\n";
            }
            // Task lists
            else if ((line.substr(0, 6) == "- [x] " || line.substr(0, 6) == "- [X] " || line.substr(0, 6) == "- [ ] "))
            {
                if (!inList)
                {
                    html += "<ul class=\"task-list\">\n";
                    inList = true;
                    listDepth = 1;
                }
                html += "  " + processTaskList(line) + "\n";
            }
            // Regular lists with nesting support
            else if (line[0] == '-' || line[0] == '*' || line[0] == '+')
            {
                int currentIndent = getIndentLevel(line);

                if (currentIndent > listDepth)
                {
                    html += "<ul>\n";
                    listDepth = currentIndent;
                    inList = true;
                }
                else if (currentIndent < listDepth)
                {
                    while (listDepth > currentIndent)
                    {
                        html += "</ul>\n";
                        listDepth--;
                    }
                }

                if (!inList)
                {
                    html += "<ul>\n";
                    inList = true;
                    listDepth = max(1, currentIndent);
                }

                string content = trim(line.substr(1));
                html += "  <li>" + processInlineFormatting(content) + "</li>\n";
            }
            // Ordered lists
            else if (isOrderedListItem(line))
            {
                closeAllLists(html);
                if (!inOrderedList)
                {
                    html += "<ol>\n";
                    inOrderedList = true;
                    listDepth = 1;
                }
                size_t dotPos = line.find(". ");
                string content = trim(line.substr(dotPos + 2));
                html += "  <li>" + processInlineFormatting(content) + "</li>\n";
            }
            // Paragraph
            else
            {
                closeAllLists(html);
                if (inTable)
                {
                    html += "</tbody>\n</table>\n";
                    inTable = false;
                }
                html += "<p>" + processInlineFormatting(line) + "</p>\n";
            }
        }

        // Clean up any open tags
        closeAllLists(html);
        if (inTable)
        {
            html += "</tbody>\n</table>\n";
        }
        if (inCodeBlock)
        {
            html += "</code></pre>\n";
            inCodeBlock = false;
        }

        // Add TOC at the beginning
        string toc = generateTOC();

        // Add footnotes at the end
        string footnotesHtml = generateFootnotes();

        return toc + html + footnotesHtml;
    }
};

bool fileExists(const string &filename)
{
    ifstream file(filename);
    return file.good();
}

string generateHTML(const string &content, const string &title = "Converted Document")
{
    return "<!DOCTYPE html>\n"
           "<html lang=\"en\">\n"
           "<head>\n"
           "  <meta charset=\"UTF-8\">\n"
           "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
           "  <title>" +
           title + "</title>\n"
                   "  <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/default.min.css\">\n"
                   "  <script src=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js\"></script>\n"
                   "  <script src=\"https://polyfill.io/v3/polyfill.min.js?features=es6\"></script>\n"
                   "  <script id=\"MathJax-script\" async src=\"https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js\"></script>\n"
                   "  <style>\n"
                   "    :root {\n"
                   "      --bg-color: #ffffff;\n"
                   "      --text-color: #333333;\n"
                   "      --border-color: #dddddd;\n"
                   "      --code-bg: #f4f4f4;\n"
                   "      --blockquote-border: #dddddd;\n"
                   "      --table-border: #dddddd;\n"
                   "    }\n"
                   "    \n"
                   "    [data-theme=\"dark\"] {\n"
                   "      --bg-color: #1a1a1a;\n"
                   "      --text-color: #e0e0e0;\n"
                   "      --border-color: #444444;\n"
                   "      --code-bg: #2d2d2d;\n"
                   "      --blockquote-border: #555555;\n"
                   "      --table-border: #555555;\n"
                   "    }\n"
                   "    \n"
                   "    body {\n"
                   "      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;\n"
                   "      max-width: 900px;\n"
                   "      margin: 0 auto;\n"
                   "      padding: 20px;\n"
                   "      line-height: 1.6;\n"
                   "      background-color: var(--bg-color);\n"
                   "      color: var(--text-color);\n"
                   "      transition: background-color 0.3s, color 0.3s;\n"
                   "    }\n"
                   "    \n"
                   "    .theme-toggle {\n"
                   "      position: fixed;\n"
                   "      top: 20px;\n"
                   "      right: 20px;\n"
                   "      background: var(--code-bg);\n"
                   "      border: 1px solid var(--border-color);\n"
                   "      border-radius: 50px;\n"
                   "      padding: 10px 15px;\n"
                   "      cursor: pointer;\n"
                   "      font-size: 16px;\n"
                   "      transition: all 0.3s;\n"
                   "    }\n"
                   "    \n"
                   "    .theme-toggle:hover {\n"
                   "      transform: scale(1.1);\n"
                   "    }\n"
                   "    \n"
                   "    code {\n"
                   "      background-color: var(--code-bg);\n"
                   "      padding: 2px 6px;\n"
                   "      border-radius: 4px;\n"
                   "      font-family: 'SF Mono', Monaco, 'Cascadia Code', 'Roboto Mono', Consolas, 'Courier New', monospace;\n"
                   "      font-size: 0.9em;\n"
                   "    }\n"
                   "    \n"
                   "    pre {\n"
                   "      background-color: var(--code-bg);\n"
                   "      padding: 16px;\n"
                   "      border-radius: 8px;\n"
                   "      overflow-x: auto;\n"
                   "      border: 1px solid var(--border-color);\n"
                   "    }\n"
                   "    \n"
                   "    pre code {\n"
                   "      background: none;\n"
                   "      padding: 0;\n"
                   "    }\n"
                   "    \n"
                   "    blockquote {\n"
                   "      border-left: 4px solid var(--blockquote-border);\n"
                   "      margin: 0;\n"
                   "      padding-left: 20px;\n"
                   "      color: var(--text-color);\n"
                   "      opacity: 0.8;\n"
                   "      font-style: italic;\n"
                   "    }\n"
                   "    \n"
                   "    hr {\n"
                   "      border: none;\n"
                   "      border-top: 2px solid var(--border-color);\n"
                   "      margin: 30px 0;\n"
                   "    }\n"
                   "    \n"
                   "    img {\n"
                   "      max-width: 100%;\n"
                   "      height: auto;\n"
                   "      border-radius: 8px;\n"
                   "      box-shadow: 0 4px 8px rgba(0,0,0,0.1);\n"
                   "    }\n"
                   "    \n"
                   "    ul, ol {\n"
                   "      padding-left: 25px;\n"
                   "    }\n"
                   "    \n"
                   "    li {\n"
                   "      margin: 8px 0;\n"
                   "    }\n"
                   "    \n"
                   "    .task-list {\n"
                   "      list-style: none;\n"
                   "      padding-left: 0;\n"
                   "    }\n"
                   "    \n"
                   "    .task-item {\n"
                   "      display: flex;\n"
                   "      align-items: center;\n"
                   "      margin: 8px 0;\n"
                   "    }\n"
                   "    \n"
                   "    .task-item input[type=\"checkbox\"] {\n"
                   "      margin-right: 8px;\n"
                   "      transform: scale(1.2);\n"
                   "    }\n"
                   "    \n"
                   "    table {\n"
                   "      width: 100%;\n"
                   "      border-collapse: collapse;\n"
                   "      margin: 20px 0;\n"
                   "      border: 1px solid var(--table-border);\n"
                   "      border-radius: 8px;\n"
                   "      overflow: hidden;\n"
                   "    }\n"
                   "    \n"
                   "    th, td {\n"
                   "      padding: 12px 15px;\n"
                   "      text-align: left;\n"
                   "      border-bottom: 1px solid var(--table-border);\n"
                   "    }\n"
                   "    \n"
                   "    th {\n"
                   "      background-color: var(--code-bg);\n"
                   "      font-weight: 600;\n"
                   "    }\n"
                   "    \n"
                   "    tr:hover {\n"
                   "      background-color: var(--code-bg);\n"
                   "    }\n"
                   "    \n"
                   "    .toc {\n"
                   "      background-color: var(--code-bg);\n"
                   "      padding: 20px;\n"
                   "      border-radius: 8px;\n"
                   "      margin-bottom: 30px;\n"
                   "      border: 1px solid var(--border-color);\n"
                   "    }\n"
                   "    \n"
                   "    .toc h2 {\n"
                   "      margin-top: 0;\n"
                   "      color: var(--text-color);\n"
                   "    }\n"
                   "    \n"
                   "    .toc ul {\n"
                   "      list-style-type: none;\n"
                   "      padding-left: 0;\n"
                   "    }\n"
                   "    \n"
                   "    .toc li {\n"
                   "      margin: 5px 0;\n"
                   "    }\n"
                   "    \n"
                   "    .toc a {\n"
                   "      text-decoration: none;\n"
                   "      color: var(--text-color);\n"
                   "      opacity: 0.8;\n"
                   "      transition: opacity 0.3s;\n"
                   "    }\n"
                   "    \n"
                   "    .toc a:hover {\n"
                   "      opacity: 1;\n"
                   "      text-decoration: underline;\n"
                   "    }\n"
                   "    \n"
                   "    .footnotes {\n"
                   "      margin-top: 40px;\n"
                   "      padding-top: 20px;\n"
                   "      border-top: 1px solid var(--border-color);\n"
                   "    }\n"
                   "    \n"
                   "    .footnote-backref {\n"
                   "      text-decoration: none;\n"
                   "      font-size: 0.8em;\n"
                   "      margin-left: 5px;\n"
                   "    }\n"
                   "    \n"
                   "    .math-inline {\n"
                   "      font-family: 'Times New Roman', serif;\n"
                   "    }\n"
                   "    \n"
                   "    .math-block {\n"
                   "      text-align: center;\n"
                   "      margin: 20px 0;\n"
                   "      font-family: 'Times New Roman', serif;\n"
                   "    }\n"
                   "    \n"
                   "    h1, h2, h3, h4, h5, h6 {\n"
                   "      margin-top: 2em;\n"
                   "      margin-bottom: 0.5em;\n"
                   "      font-weight: 600;\n"
                   "    }\n"
                   "    \n"
                   "    h1 { font-size: 2.2em; }\n"
                   "    h2 { font-size: 1.8em; }\n"
                   "    h3 { font-size: 1.5em; }\n"
                   "    h4 { font-size: 1.3em; }\n"
                   "    h5 { font-size: 1.1em; }\n"
                   "    h6 { font-size: 1em; }\n"
                   "    \n"
                   "    a {\n"
                   "      color: #0066cc;\n"
                   "      text-decoration: none;\n"
                   "    }\n"
                   "    \n"
                   "    a:hover {\n"
                   "      text-decoration: underline;\n"
                   "    }\n"
                   "    \n"
                   "    [data-theme=\"dark\"] a {\n"
                   "      color: #66b3ff;\n"
                   "    }\n"
                   "  </style>\n"
                   "</head>\n"
                   "<body>\n"
                   "  <button class=\"theme-toggle\" onclick=\"toggleTheme()\">🌓</button>\n"
                   "  \n" +
           content + "\n"
                     "  \n"
                     "  <script>\n"
                     "    // Initialize syntax highlighting\n"
                     "    hljs.highlightAll();\n"
                     "    \n"
                     "    // Configure MathJax\n"
                     "    window.MathJax = {\n"
                     "      tex: {\n"
                     "        inlineMath: [['$', '$']],\n"
                     "        displayMath: [['$$', '$$']]\n"
                     "      }\n"
                     "    };\n"
                     "    \n"
                     "    // Dark mode toggle\n"
                     "    function toggleTheme() {\n"
                     "      const body = document.body;\n"
                     "      const currentTheme = body.getAttribute('data-theme');\n"
                     "      body.setAttribute('data-theme', currentTheme === 'dark' ? 'light' : 'dark');\n"
                     "      localStorage.setItem('theme', body.getAttribute('data-theme'));\n"
                     "    }\n"
                     "    \n"
                     "    // Persist theme across reloads\n"
                     "    (function() {\n"
                     "      const savedTheme = localStorage.getItem('theme') || 'light';\n"
                     "      document.body.setAttribute('data-theme', savedTheme);\n"
                     "    })();\n"
                     "  </script>\n"
                     "</body>\n"
                     "</html>";
}

int main(int argc, char *argv[])
{
    string inputFile = "input.md";
    string outputFile = "output.html";

    // Command line arguments
    if (argc >= 2)
        inputFile = argv[1];
    if (argc >= 3)
        outputFile = argv[2];

    // Check if input file exists
    if (!fileExists(inputFile))
    {
        cerr << "Error: Input file '" << inputFile << "' not found.\n";
        cerr << "Usage: " << argv[0] << " [input.md] [output.html]\n";
        return 1;
    }

    ifstream inFile(inputFile);
    ofstream outFile(outputFile);

    if (!inFile.is_open())
    {
        cerr << "Error: Cannot open input file '" << inputFile << "'.\n";
        return 1;
    }

    if (!outFile.is_open())
    {
        cerr << "Error: Cannot create output file '" << outputFile << "'.\n";
        return 1;
    }

    // Read entire file
    ostringstream buffer;
    buffer << inFile.rdbuf();
    string markdown = buffer.str();

    // Convert markdown to HTML
    MarkdownConverter converter;
    string htmlContent = converter.convertToHTML(markdown);

    // Generate complete HTML document
    string fullHTML = generateHTML(htmlContent, "Markdown Document");

    // Write to output file
    outFile << fullHTML;

    cout << "Conversion complete!\n";
    cout << "  Input:  " << inputFile << " (" << markdown.length() << " characters)\n";
    cout << "  Output: " << outputFile << "\n";

    inFile.close();
    outFile.close();

    return 0;
}