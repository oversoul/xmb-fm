# XMB File Manager

an attempt to make a file manager with a UI resembling XMB (XrossMediaBar) more known as PS3 UI.

it's using OpenGL, FreeType, GLFW

# Keybindings

- Arrow Keys: `up` and `down` for vertical menu, `left` & `right` for horizontal menu.
- `Enter`: open directory, open file using `xdg-open` (not fully tested).
- `Backspace`: navigate to parent directory.
- `i`: to open options list
- `p`: preview first 500 char of a file if detected as text file.
- `+`/`-`: switch the themes
- `PAGE_UP`/`PAGE_DOWN`: scroll up and down by 10.
- `HOME`/`END`: scroll to start / end of list.
- `/`: show a search input, after validating `Enter` it will scroll to item that contains keyword.
