# type: ignore
# pylint: skip-file

# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
from selfdrive.version import get_version 
sys.path.insert(0, os.path.abspath('.'))
sys.path.insert(0, os.path.abspath('..'))
VERSION = get_version()


# -- Project information -----------------------------------------------------

project = 'openpilot docs'
copyright = '2021, comma.ai'
author = 'comma.ai'
version = VERSION
release = VERSION
language = 'en'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
        'sphinx.ext.autodoc',   # Auto-generate docs
        'sphinx.ext.viewcode',  # Add view code link to modules
        'sphinx_rtd_theme',     # Read The Docs theme
        'myst_parser',          # Markdown parsing
        'sphinx_sitemap',       # sitemap generation for SEO
]

myst_html_meta = {
   "description": "openpilot docs",
   "keywords": "op, openpilot, docs, documentation",
   "robots": "all,follow",
   "googlebot": "index,follow,snippet,archive",
   "property=og:locale": "en_US",
   "property=og:site_name": "docs.comma.ai",
   "property=og:url": "https://docs.comma.ai",
   "property=og:title": "openpilot Docuemntation",
   "property=og:type": "website",
   "property=og:image:type": "image/jpeg",
   "property=og:image:width": "400",
   "property=og:image": "https://docs.comma.ai/_static/logo.png",
   "property=og:image:url": "https://docs.comma.ai/_static/logo.png",
   "property=og:image:secure_url": "https://docs.comma.ai/_static/logo.png",
   "property=og:description": "openpilot Documentation",
   "property=twitter:card": "summary_large_image",
   "property=twitter:logo": "https://docs.comma.ai/_static/logo.png",
   "property=twitter:title": "openpilot Documentation",
   "property=twitter:description": "openpilot Documentation"
}

html_baseurl = 'https://docs.comma.ai/'
sitemap_filename = "sitemap.xml"

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'
html_show_copyright = True

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']
html_logo = '_static/logo.png'
html_favicon = '_static/favicon.ico'
html_theme_options = {
    'logo_only': False,
    'display_version': True,
    'vcs_pageview_mode': 'blob',
    'style_nav_header_background': '#000000',
}
html_extra_path = ['_static']
