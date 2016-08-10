#!/usr/bin/env qore

%requires xml
%requires Qorize
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
                res.value = "abc";
                break;
            case "anyUri":
                res.type = "string";
                res.value = "http://www.qore.com";
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
                res.value = 123;
                break;
            case "boolean":
                res.type = "bool";
                res.value = True;
                break;
            case "base64Binary":
            case "hexBinary":
                res.type = "binary";
                res.value = "<0feba023ffdca6291>";   # TODO: how assign binary const ?
                res.xvalue = "<0feba023ffdca6291>";
                res.svalue = res.xvalue;
                break;
            case "decimal":
                res.type = "number";
                res.value = 123.456;
                break;
            case "date":
                res.type = "date";
                res.value = 2015-01-31;
                res.xvalue = format_date('YYYY-MM-DD', res.value);
                break;
            case "dateTime":
                res.type = "date";
                res.value = 2015-01-31T10:20:30+01:00;
                res.xvalue = format_date('YYYY-MM-DDTHH:mm:SSZ', res.value);
                break;
            case "time":
                res.type = "date";
                res.value = 10:20:30Z;
                res.xvalue = format_date('HH:mm:SSZ', res.value);
                break;
            case "duration":
                res.type = "date";
                res.value = P5Y2M10DT15H;   # TODO
                break;
        }
        if (res.type == "string") {
            #res.svalue = "'"+res.value+"'";
            res.xvalue = res.value;
        }
        if (!exists res.svalue) {
            res.svalue = sprintf('%y', res.value);
        }
        if (!exists res.xvalue) {
            res.xvalue = sprintf('%y', res.value);
        }
        return res;
    }

    private hash getTypeInfo(XsdSimpleType t) {
        my hash res;
        if (t.type) {
            res = getTypeInfo(t.type);
            res.xtype = t.name;
            if (t.enum) {
                switch (res.type) {
                case "string":
                    res.value = (keys t.enum)[0];
                    res.svalue = sprintf("'%s'", res.value);
                    break;
                case "int":
                    res.value = sprintf("%s", (keys t.enum)[0]);
                    res.svalue = string(res.value);
                    break;
                default:
                    res.value = sprintf("%s", (keys t.enum)[0]);
                }
                res.xvalue = "<!-- " + (keys t.enum).join(',') + " -->";
                res.svalue += sprintf("  /* %s */",  (keys t.enum).join(','));
                res.comment = sprintf("Enum: %s",  (keys t.enum).join(','));
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
                    result += vi.svalue;
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
                    result += sprintf("%s\"%s\": %s,\n", getPrefix(indent), name2, vi2.svalue);
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

    hash getMessage(XsdElement elem, bool add_comments = True, bool string_values = False, bool only_one_choice = False) {
        my list comments = ();
        my hash vi = getTypeInfo(elem.type);
        my any val;

        if (add_comments && elem.minOccurs == 0) {
            comments += "optional";
        }
        if (vi.type) {
            if (vi.type == 'hash') {
                val = hash();
                foreach my string name2 in (keys elem.type.elementmap) {
                    val += getMessage(elem.type.elementmap{name2}, add_comments, string_values, only_one_choice);
                }
                if (elem.type.choices) {
                    my int j = 1;
                    foreach my string name2 in (keys elem.type.choices[0].elementmap) {
                        if (add_comments) {
                            val{sprintf('^comment%d^', j)} = sprintf("choice [%d]", j);
                        }
                        val += getMessage(elem.type.choices[0].elementmap{name2}, add_comments, string_values, only_one_choice);
                        j++;
                        if (only_one_choice) {
                            break;
                        }
                    }
                }
            } else {
                if (add_comments && vi.comment) {
                    comments += vi.comment;
                }
                val = string_values ? vi.xvalue : vi.value;
            }
        }

        my any v;
        if (comments || vi.attrs) {
            v = hash();
            if (comments) {
                v{'^comment^'} = comments.join(';');
            }
            if (vi.attrs) {
                v{'^attributes^'} = ();
                foreach my name2 in (keys elem.type.attrs) {
                    my hash vi2 = getTypeInfo(elem.type.attrs{name2}.type);
                    if (vi2.type) {
                        v{'^attributes^'}{name2} = string_values ? vi2.xvalue : vi2.value;
                    }
                }
            }
            if (exists val) {
                if (vi.type == 'hash') {
                    v += val;
                } else {
                    v{'^value^'} = val;
                }
            }
        } else {
            v = val;
        }
        my n = elem.maxOccurs;
        if (n == -1 || n > 3) {
            n = 3;
        }
        if (n > 1) {
            my list l;
            while (n > 0) {
                push l, v;
                n--;
            }
            return (elem.name: l);
        } else {
            return (elem.name: v);
        }
    }

    hash getMessage(string name, bool add_comments = True, bool string_values = False, bool only_one_choice = False) {
        return getMessage(ws.idocmap{name}, add_comments, string_values, only_one_choice);
    }

    hash getMessage(WSMessage msg, bool add_comments = True, bool string_values = False, bool only_one_choice = False) {
        return getMessage(msg.args.firstValue().element, add_comments, string_values, only_one_choice);
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

    string getOperationMessageAsXml(*list names, bool request) {
        if (!names) {
            names = keys ws.opmap;
        }
        my string result;
        foreach my string name in (names) {
            my WSOperation op = ws.opmap{name};
            if (request) {
                #my hash msg = getMessage((keys op.input.args)[0], False, False);
                if (op.input) {
                    my hash msg = getMessage(op.input, False, False, True);
#printf("%N\n", msg);
#printf("%N\n", op.input);
                    result += op.serializeRequest(msg, NOTHING, NOTHING, NOTHING, NOTHING, XGF_ADD_FORMATTING).body;
                }
            } else {
                if (op.output) {
#                my hash msg = getMessage((keys op.output.args)[0], False, False);
                    my hash msg = getMessage(op.output, False, False, True);
#printf("%N\n", msg);
                    result += op.serializeResponse(msg, NOTHING, NOTHING, NOTHING, NOTHING, XGF_ADD_FORMATTING).body;
#printf("%N\n", result);
                }
            }
            result += "\n\n";
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
            'print_msg': True,
            'qorize_msg': True,
            'qore_msg': True,
            'xml_msg': True,
            'dump_wsdl': True,
            'dump_ws': True,
            'xml_oper_req': True,
            'xml_oper_resp': True,
        );
        bool verbose = False;
        WebService ws;
        WSDLHelper wh;

    }

    constructor () {
        my GetOpt g(opts);
        my hash opt = g.parse3(\ARGV);
        my *list objs;
        my string operation;
        try {
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
            operation = shift l;
            objs = l ? (shift l).split(','):NOTHING;

            if (!exists operations{operation}) {
                throw "WSDL-TOOL-ERROR", sprintf("Operation '%s' is not in the list of supported operations %y\n", operation, keys operations);
            }

        } catch (e) {
            if (e.type == Qore::ET_User) {
                stderr.printf("%s\n\n%N", e.desc, e);
                help();
            } else {
                rethrow;
            }
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
        case 'print_msg':
            # temporary debug
            if (!objs) {
                objs = keys ws.idocmap;
            }
            my hash vars;
            foreach my string name in (objs) {
                vars{name} = wh.getMessage(name);
            }
            output.printf("%N\n", vars);
            break;
        case 'qorize_msg':
            # temporary debug
            if (!objs) {
                objs = keys ws.idocmap;
            }
            my hash vars;
            foreach my string name in (objs) {
                output.print(qorize(wh.getMessage(name), name, True));
            }
#            output.printf("%N\n", vars);
            break;

        case 'qore_msg':
            info("getMessagesAsQore(%y)\n", objs);
            output.print(wh.getMessageAsQore(objs));
            break;
        case 'xml_msg':
            info("getMessageAsXml(%y)\n", objs);
            output.print(wh.getMessageAsXml(objs));
            break;

        case 'xml_oper_req':
            info("getOperationMessageAsXml(%y, True)\n", objs);
            output.print(wh.getOperationMessageAsXml(objs, True));
            break;
        case 'xml_oper_resp':
            info("getOperationMessageAsXml(%y, False)\n", objs);
            output.print(wh.getOperationMessageAsXml(objs, False));
            break;
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
            "    print_msg   print message as printf output\n"
            "    qorize_msg  print message as Qorize output\n"
            "    xml_oper_req  print XML message of operation request\n"
            "    xml_oper_resp  print XML message of operation response\n"

            "    qore_msg    print message as Qore hash\n"
            "    xml_msg     print message as XML structure\n"
            "    dump_wsdl   dump input WSDL file\n"
            "    dump_ws     dump WebService hash\n"
            "    xml_oper_req  print XML message of operation request\n"
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
