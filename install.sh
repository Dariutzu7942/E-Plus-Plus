#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
install_prefix=${EPP_INSTALL_PREFIX:-"$HOME/.local"}

cmake -S "$repo_dir/E++" -B "$repo_dir/build-language" -DCMAKE_INSTALL_PREFIX="$install_prefix"
cmake --build "$repo_dir/build-language"
cmake --install "$repo_dir/build-language"

cmake -S "$repo_dir/E++ GUI" -B "$repo_dir/build-gui" -DCMAKE_INSTALL_PREFIX="$install_prefix"
cmake --build "$repo_dir/build-gui"
cmake --install "$repo_dir/build-gui"

if command -v code >/dev/null 2>&1 && [ -f "$repo_dir/epp-language-support-0.1.0.vsix" ]; then
	code --install-extension "$repo_dir/epp-language-support-0.1.0.vsix"
fi

printf 'Installed epp and epp-gui into %s/bin\n' "$install_prefix"
printf 'Add this to your shell profile if needed: export PATH="%s/bin:$PATH"\n' "$install_prefix"
