#!/usr/bin/env qore

%requires xml
%requires ../qlib/WSDL.qm
%new-style

/*
./wsdl-test.q qore_msg -v -w /home/tma/omq/wsdl/msepl_at/wincash/MaterialMovementInterface_v13.wsdl

/home/tma/omq/wsdl/msepl_at/wincash/
BasketInterface_v13.wsdl
DbodInterface_v13.wsdl
EmployeeInterface_v13.wsdl
MaterialMovementInterface_v13.wsdl
SalesDataInterface_v13.wsdl

/home/tma/work/qore/module-xml/examples/wsdl-viewer/test/TimeService.wsdl

TODO: 
    use bindings to print SOAPClient/server code
    human readable report

*/

class WSDLHelper {
    const ALL = '*';

    private {
        WebService ws;
        int xmlIndentation = 2;
        int qoreIndentation = 4;
    }

    constructor (WebService _ws) {
        ws = _ws;

    }

    private hash getTypeInfo(XsdBaseType t) {
        my hash res = (
            'xtype': t.name,
        );
        switch (t.name) {
            case "string":
            case "normalizedString":
            case "token":
                res.type = "string";
                res.xvalue = "abc";
                break;
            case "anyUri":
                res.type = "string";
                res.xvalue = "http://www.qore.com";
                break;
            case "byte":
            case "int":
            case "integer":
            case "long":
            case "negativeInteger":
            case "nonNegativeInteger":
            case "positiveInteger":
            case "nonPositiveInteger":
            case "short":
            case "unsignedLong":
            case "unsignedInt":
            case "unsignedShort":
            case "unsignedByte":
                res.type = "int";
                res.value = "123";
                break;
            case "boolean":
                res.type = "bool";
                res.value = "true";
                break;
            case "base64Binary":
            case "hexBinary":
                res.type = "binary";
                res.value = "<0feba023ffdca6291>";
                break;
            case "decimal":
                res.type = "number";
                res.value = "123.456";
                break;
            case "date":
                res.type = "date";
                res.value = "2015/01/31";
                break;
            case "dateTime":
                res.type = "date";
                res.value = "2015/01/31T10:20:30+01:00";
                break;
            case "time":
                res.type = "date";
                res.value = "10:20:30";
                break;
            case "duration":
                res.type = "date";
                res.value = "P5Y2M10DT15H";   # TODO
                break;
        }
        if (res.type == "string") {
            res.value = "'"+res.xvalue+"'";
        }
        if (!exists res.xvalue) {
            res.xvalue = res.value;
        }
        return res;
    }

    private hash getTypeInfo(XsdSimpleType t) {
        my hash res;
        if (t.type) {
            res = getTypeInfo(t.type);
            res.xtype = t.name;
            if (t.enum) {
                if (res.type == "string" ) {
                    res.value = sprintf("'%s'", (keys t.enum)[0]);
                } else {
                    res.value = sprintf("%s", (keys t.enum)[0]);
                }
                res.xvalue = "<!-- " + (keys t.enum).join(',') + " -->";
                res.value += sprintf("  # " + (keys t.enum).join(','));
            }
        } else {
            res.xtype = t.name;
        }
        return res;
    }

    private hash getTypeInfo(XsdComplexType t) {
        my hash res;
        if (t.elementmap) {
            res.type = "hash";
        } else if (t.simpleType) {
            res = getTypeInfo(t.simpleType);
        }
        res.attrs = exists t.attrs;
        return res;
    }

    private string getPrefix(int indent) {
        return strmul(' ', indent, NOTHING);
    }

    # cursor position is set to position of next output
    private string getMessageAsQoreIntern(string fmt_name, XsdElement elem, int indent) {
        my string result = '';
        if (elem.minOccurs == 0) {
            result += getPrefix(indent) + "# optional\n";
        }
        result += getPrefix(indent) + fmt_name;
        my n = elem.maxOccurs > 0 ? elem.maxOccurs : 1;
#printf("%y\n", fmt_name);
        my hash vi = getTypeInfo(elem.type);
        if (n > 1) {
            indent += qoreIndentation;
            result += "\n"+getPrefix(indent)+"(\n";
            indent += qoreIndentation;
        }
        if (vi.attrs) {
            indent += qoreIndentation;
            result += "\n"+getPrefix(indent)+"{\n";
            indent += qoreIndentation;
            if (vi.type) {
                result += getPrefix(indent)+"\"^value^\":";
            }
        }
        if (vi.type) {
            for (my int i = 0; i < n; i++) {
                if (vi.type == 'hash') {
                    indent += qoreIndentation;
                    result += "\n"+getPrefix(indent)+"{\n";
                    foreach my string name2 in (keys elem.type.elementmap) {
                        result += getMessageAsQoreIntern(sprintf("\"%s\": ", name2), elem.type.elementmap{name2}, indent+qoreIndentation);
                        result += ",\n";
                    }
                    if (elem.type.choices) {
                        indent += qoreIndentation;
                        result += getPrefix(indent)+"# <choices>\n";
                        indent += qoreIndentation;
                        my int j = 1;
                        foreach my string name2 in (keys elem.type.choices[0].elementmap) {
                            result += getPrefix(indent)+sprintf("# choice [%d]\n", j);
                            result += getMessageAsQoreIntern(sprintf("\"%s\": ", name2), elem.type.choices[0].elementmap{name2}, indent);
                            result += ",\n";
                            j++;
                        }
                        indent -= qoreIndentation;
                        result += getPrefix(indent)+"# </choices>\n";
                        indent -= qoreIndentation;
                    }
                    result += getPrefix(indent)+"}";
                    indent -= qoreIndentation;
                } else {
                    result += vi.value;
                }

                if (n > 1) {
                    result += ",\n";
                }
                if (i > 3) {
                    result += sprintf("\n%s# ... %d items\n", getPrefix(indent), elem.maxOccurs);
                    break;
                }
            }
        }
        if (vi.attrs) {
            if (vi.type) {
                result += ",\n";
            }
            result += getPrefix(indent)+"\"^attributes^\":";
            indent += qoreIndentation;
            result += "\n"+getPrefix(indent)+"{\n";
            indent += qoreIndentation;

            foreach my name2 in (keys elem.type.attrs) {
                my hash vi2 = getTypeInfo(elem.type.attrs{name2}.type);
                if (vi2.type) {
                    result += sprintf("%s\"%s\": %s,\n", getPrefix(indent), name2, vi2.value);
                }
            }

            indent -= qoreIndentation;
            result += getPrefix(indent)+"}";
            indent -= qoreIndentation;

            indent -= qoreIndentation;
            result += "\n"+getPrefix(indent)+"}";
            indent -= qoreIndentation;
        }
        if (n > 1) {
            indent -= qoreIndentation;
            result += getPrefix(indent) + ")";
            indent -= qoreIndentation;
        }
        return result;
    }

    string getMessageAsQore(*list names) {
        if (!names) {
            names = keys ws.idocmap;
        }
        my string result;
        foreach my string name in (names) {
            my XsdElement elem = ws.idocmap{name};
            if (elem.maxOccurs > 1) {
                result += getMessageAsQoreIntern(sprintf("my list %s = ", name), elem, 0);
            } else {
                my hash vi = getTypeInfo(elem.type);
                result += getMessageAsQoreIntern(sprintf("my %s %s = ", vi.type, name), elem, 0);
            }
            result += ";\n\n";
        }
        return result;
    }

    private string getMessageAsXmlIntern(string name, XsdElement elem, reference nsc, int indent) {
        my string result = '';
        /*if (elem.minOccurs == 0) {
            result += getPrefix(indent) + "# optional\n";
        }*/
        my hash vi = getTypeInfo(elem.type);
        my string elem_name = name;
        if (elem.ns) {
            ws.nsc.pushTargetNamespace(elem.ns);
            nsc{ws.nsc.getTargetNamespaceInputPrefix()} = elem.ns;
            elem_name = ws.nsc.getTargetNamespaceInputPrefix()+":"+elem_name;
            ws.nsc.popTargetNamespace();
        }

        result += getPrefix(indent) + "<" + elem_name;
        if (vi.attrs) {
            foreach my name2 in (keys elem.type.attrs) {
                my hash vi2 = getTypeInfo(elem.type.attrs{name2}.type);
                if (vi2.type) {
                    result += sprintf(" %s=\"%s\"", name2, vi2.xvalue);
                }
            }
        }
        result += ">\n";
        indent += xmlIndentation;
        if (vi.xvalue) {
            result += getPrefix(indent) + vi.xvalue + "\n";
        }
        if (vi.type == "hash") {
            foreach my string name2 in (keys elem.type.elementmap) {
                result += getMessageAsXmlIntern(name2, elem.type.elementmap{name2}, \nsc, indent);
            }
            if (elem.type.choices) {
                my int j = 1;
                foreach my string name2 in (keys elem.type.choices[0].elementmap) {
                    result += getPrefix(indent)+sprintf("<!--choice [%d]-->\n", j);
                    result += getMessageAsXmlIntern(name2, elem.type.choices[0].elementmap{name2}, \nsc, indent);
                    j++;
                }
            }
        }
        indent -= xmlIndentation;
        result += getPrefix(indent) + "</" + elem_name + ">\n";

        return result;
    }

    string getMessageAsXml(*list names) {
        if (!names) {
            names = keys ws.idocmap;
        }
        my string result;
        foreach my string name in (names) {
            hash nsc = {};
            my string body = getMessageAsXmlIntern(name, ws.idocmap{name}, \nsc, 4);
            my string namespaces = "";
            foreach my ns in (keys nsc) {
                namespaces += sprintf(" xmlns:%s=\"%s\"", ns, nsc{ns});
            }
            result +=
'<?xml version="1.0"?>
<soap:Envelope xmlns:soap="http://www.w3.org/2003/05/soap-envelope/" soap:encodingStyle="http://www.w3.org/2003/05/soap-encoding"' + namespaces + '>
  <soap:Header>
  </soap:Header>

  <soap:Body>
' + body + '
    <soap:Fault>
    </soap:Fault>
  </soap:Body>
</soap:Envelope>

';
        }
        return result;
    }

}


%exec-class WSDLTools
class WSDLTools {
    private {
        hash opts = (
            'help': 'h',
            'verbose': 'v',
            'wsdl': 'w=s',
            'output': 'o=s',
        );
        hash operations = (
            'qore_msg': True,
            'xml_msg': True,
            'dump_wsdl': True,
            'dump_ws': True,
        );
        bool verbose = False;
        WebService ws;
        WSDLHelper wh;

    }

    constructor () {
        try {
            my GetOpt g(opts);
            my hash opt = g.parse3(\ARGV);
            if (opt.help) {
                help();
            }
            if (opt.verbose) {
                verbose = True;
            }
            if (ARGV.empty()) {
                throw "WSDL-TOOL-ERROR", "No operation specified";
            }
            my list l = (shift ARGV).split(':');
            if (!ARGV.empty()) {
                throw "WSDL-TOOL-ERROR", "Only one operation allowed";
            }
            my string operation = shift l;
            *list objs = l ? (shift l).split(','):NOTHING;

            if (!exists operations{operation}) {
                throw "WSDL-TOOL-ERROR", sprintf("Operation '%s' is not in the list of supported operations %y\n", operation, keys operations);
            }

            my *string wsdlContent;
            if (!opt.wsdl) {
                wsdlContent = stdin.read(-1);
            } else {
                string url = opt.wsdl;
                if (! (url =~ /:\/\//)) {
                    url = 'file://'+url;
                }
                info("getFileFromURL(%s)\n", url);
                hash h = Qore::parse_url(url);
                wsdlContent = WSDLLib::getFileFromURL(url, h, 'file', NOTHING, NOTHING, True);
            }

            my File output = stdout;
            if (exists opt.output) {
                output = new File();
                output.open(opt.output, O_CREAT | O_TRUNC | O_WRONLY);
            }

            if (operation == 'dump_wsdl') {
                output.print(wsdlContent);
                return;
            }

            ws = new WebService(wsdlContent);

            if (operation == 'dump_ws') {
                ws.wsdl = '';
                output.printf("%N\n", ws);
                return;
            }
            wh = new WSDLHelper(ws);

            switch (operation) {
            case 'qore_msg':
                info("getMessagesAsQore(%y)\n", objs);
                output.print(wh.getMessageAsQore(objs));
                break;
            case 'xml_msg':
                info("getMessageAsXml(%y)\n", objs);
                output.print(wh.getMessageAsXml(objs));
                break;
            }
        } catch (e) {
            if (e.type == Qore::ET_User) {
                stderr.printf("%s\n\n", e.desc);
                help();
            } else {
                rethrow;
            }
        }
    }

    private help() {
        stderr.printf(
            "WSDL tools, ver 0.1\n"
            "usage: %s [options] [-w wsdl] [-o output] operation[:name[,name...]]\n"
            "  options\n"
            "  -v     verbose\n"
            "  -h     help\n"
            "  wsdl   WSDL file or URL. Unless specified then expected at STDIN\n"
            "  output unless file specified then goes to STDOUT\n"
            "  operation\n"
            "    qore_msg    print message as Qore hash\n"
            "    xml_msg     print message as XML structure\n"
            "    dump_wsdl   dump input WSDL file\n"
            "    dump_ws     dump WebService hash\n"
            "  name   output object name. If not specified then output all objects\n"
            "\n"
            "Example:\n"
            "  %s qore_msg MaterialMovementInterface_v13.wsdl\n"
            "\n"
        ,
            get_script_name(),
            get_script_name(),
        );
        exit(1);
    }

    private info(string s, p) {
        if (verbose) {
            stderr.vprintf(s, p);
        }
    }
}
