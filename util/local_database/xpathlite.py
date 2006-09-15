import sys
import os
import xml.dom.minidom

doc_cache = {}

def findChild(parent, tag_name, arg_value):
    for node in parent.childNodes:
        if node.nodeType != node.ELEMENT_NODE:
            continue
        if node.nodeName != tag_name:
            continue
        if arg_value:
            if not node.attributes.has_key('type'):
                continue
            if node.attributes['type'].nodeValue != arg_value:
                continue
        return node
    return False

def _findEntry(file, path):
    doc = False
    if doc_cache.has_key(file):
        doc = doc_cache[file]
    else:
        doc = xml.dom.minidom.parse(file)
        doc_cache[file] = doc

    elt = doc.documentElement
    tag_spec_list = path.split("/")
    for tag_spec in tag_spec_list:
        tag_name = tag_spec
        arg_value = ''
        left_bracket = tag_spec.find('[')
        if left_bracket != -1:
            tag_name = tag_spec[:left_bracket]
            arg_value = tag_spec[left_bracket+1:-1]
        elt = findChild(elt, tag_name, arg_value)
        if not elt:
            return ""

    return elt.firstChild.nodeValue

def findAlias(file):
    doc = doc_cache[file]
    alias_elt = findChild(doc.documentElement, "alias", "")
    if not alias_elt:
        return False
    if not alias_elt.attributes.has_key('source'):
        return False
    return alias_elt.attributes['source'].nodeValue

def findEntry(base, path):
    file = base + ".xml"

    if os.path.isfile(file):
        result = _findEntry(file, path)
        if result:
            return result

        alias = findAlias(file)
        if alias:
            file = os.path.dirname(base) + "/" + alias + ".xml"
            if os.path.isfile(file):
                result = _findEntry(file, path)
                if result:
                    return result

    file = base[:-3] + ".xml"
    if os.path.isfile(file):
        result = _findEntry(file, path)
        if result:
            return result
        alias = findAlias(file)
        if alias:
            file = os.path.dirname(base) + "/" + alias + ".xml"
            if os.path.isfile(file):
                result = _findEntry(file, path)
                if result:
                    return result

    file = os.path.dirname(base) + "/root.xml"
    result = _findEntry(file, path)
    return result

