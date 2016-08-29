#!/usr/bin/env qore

%requires xml
%requires Qorize
%requires ../qlib/WSDL.qm
%new-style

/*
./wsdl-test.q qore_msg -v MaterialMovementInterface_v13.wsdl msg exportMaterialMovementData response -f human

/home/tma/omq/wsdl/msepl_at/wincash/
BasketInterface_v13.wsdl
DbodInterface_v13.wsdl
EmployeeInterface_v13.wsdl
MaterialMovementInterface_v13.wsdl
SalesDataInterface_v13.wsdl

/home/tma/work/qore/module-xml/examples/wsdl-viewer/test/TimeService.wsdl

*/

class WSDLHelper {

    private {
        WebService ws;
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
                    res.svalue = sprintf("%y", res.value);
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
                foreach my hash choice in (elem.type.choices) {
                    my int j = 1;
                    foreach my string name2 in (keys choice.elementmap) {
                        if (add_comments) {
                            val{sprintf('^comment%d^', j)} = sprintf("choice [%d]", j);
                        }
                        val += getMessage(choice.elementmap{name2}, add_comments, string_values, only_one_choice);
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

    private getOperationParams(WSMessage msg) {
        return sprintf("%s(%s)", msg.name, (keys msg.args).join(','));
    }
    nothing makeReport(File output, string wsdl_name) {
        output.printf("wsdl: %s\n", wsdl_name);
        foreach my hash svc in (ws.listServices()) {
            output.printf("  service: %s\n", svc.name);
            #my hash svc = ws.getService(svc_name);
            foreach my string port in (keys svc.port) {
                output.printf("    port: %s\n", port);
                output.printf("      binding: %s\n", svc.port{port}.binding.name);
                output.printf("      address: %s\n", svc.port{port}.address);
                my hash portType = ws.portType{svc.port{port}.binding.getPort()};
            output.print("      operations:\n");
                foreach my string name in (keys portType.operations) {
                    my WSOperation op = portType.operations{name};
                    output.printf("        %s\n", name);
                    if (op.input) {
                        output.printf("          input: %s\n", getOperationParams(op.input));
                    }
                    if (op.output) {
                        output.printf("          output: %s\n", getOperationParams(op.output));
                    }
                }
            }
        }
    }
}


%exec-class WSDLTools
class WSDLTools {
    private {
        const MSG_REQUEST = 0x1;
        const MSG_RESPONSE = 0x2;
        hash opts = (
            'help': 'h',
            'verbose': 'v:i+',
            'output': 'o=s',
        );
        hash operations = (
            'report': (
            ),
            'msg': (
                'opts': (
                    'format': 'f=s',
                    'comment': 'c',
                    'choices': 'm',
                ),
                'params': (
                    True,
                    ('both': MSG_REQUEST|MSG_RESPONSE, 'request': MSG_REQUEST, 'response': MSG_RESPONSE, ),
                ),
                'formats': ('human': True, 'xml': True, 'qore': True, ),
            ),
            'dump': (
                'params': (
                    ('ws': 'ws', 'wsdl': 'wsdl', ),
                ),
            ),
        );
        int verbose = 0;
        WebService ws;
        WSDLHelper wh;
    }

    constructor () {
        my GetOpt g(opts);
        # first try to get general options (help, etc.), do not modify ARGV
        *softlist argv2 = ARGV;
        my hash opt = g.parse(\ARGV);
        my *list objs;
        my string operation;
        my string wsdl_file;
        my softlist params;
        try {
            if (opt.help) {
                help();
            }
            verbose = opt.verbose ?? verbose;
            if (ARGV.empty()) {
                throw "WSDL-TOOL-ERROR", "No WSDL file provided";
            }
            wsdl_file = shift ARGV;

            if (ARGV.empty()) {
                operation = (keys operations)[0];
            } else {
                operation = shift ARGV;
                if (!exists operations{operation}) {
                    throw "WSDL-TOOL-ERROR", sprintf("Operation %y is not in the list of supported operations %y", operation, keys operations);
                }
            }
            if (operations{operation}.opts) {
                g = new GetOpt(opts + operations{operation}.opts);
            }
            # check all options
            ARGV = argv2;
            opt = g.parse3(\ARGV);
            # it might be tricky as changes options may change eaten params
            shift ARGV;  # remove wdsl
            if (!ARGV.empty()) {
                shift ARGV;  # remove operation
            }

            int n = exists operations{operation}.params ? operations{operation}.params.size() : 0;
            if (ARGV.size()-2 > n) {
                throw "WSDL-TOOL-ERROR", sprintf("Operation %y supports up to %y parameters but provided %y", operation, n, ARGV);
            }
            for (my i = 0; i < n; i++) {
                if (operations{operation}.params[i].typeCode() == NT_HASH) {
                    if (i >= ARGV.size()) {
                        push params, operations{operation}.params[i].firstValue();
                    } else {
                        if (exists operations{operation}.params[i]{ARGV[i]}) {
                            push params, operations{operation}.params[i]{ARGV[i]};
                        } else {
                            throw "WSDL-TOOL-ERROR", sprintf("Wrong parameter value %y supported values %y", ARGV[i], keys operations{operation}.params[i]);
                        }
                    }
                } else {
                    push params, ARGV[i] ?? '';
                }
            }
            switch (operation) {
            case 'report':
                break;
            case 'msg':
                if (opt.format) {
                    if (! exists operations{operation}.formats{opt.format}) {
                        throw "WSDL-TOOL-ERROR", sprintf("Value %y is not in the list of supported formats %y\n", opt.format, keys operations{operation}.formats);
                    }
                } else {
                    opt.format = operations{operation}.formats.firstKey();
                }
                break;
            case 'dump':
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
        *hash ws_opts;
        my *string wsdlContent;
        if (wsdl_file == '-') {
            wsdlContent = stdin.read(-1);
        } else {
            string url = wsdl_file;
            if (! (url =~ /:\/\//)) {
                url = 'file://'+url;
                ws_opts = ("def_path": dirname(wsdl_file));
            }
            else {
                string dir = dirname(wsdl_file);
                dir =~ s/^.*:\/\///;
                ws_opts = ("def_path": dir);
            }

            info(sprintf("getFileFromURL(%y)\n", url));
            hash h = Qore::parse_url(url);
            wsdlContent = WSDLLib::getFileFromURL(url, h, 'file', NOTHING, NOTHING, True);
        }

        my File output = stdout;
        if (exists opt.output) {
            output = new File();
            output.open(opt.output, O_CREAT | O_TRUNC | O_WRONLY);
        }

        if (operation == 'dump' && params[0] == 'wsdl') {
            output.print(wsdlContent);
            return;
        }

        ws = new WebService(wsdlContent, ws_opts);

        if (operation == 'dump' && params[0] == 'ws') {
            ws.wsdl = '';
            output.printf("%N\n", ws);
            return;
        }
        wh = new WSDLHelper(ws);

        switch (operation) {
            case 'report':
                info(sprintf("%s()\n", operation));
                wh.makeReport(output, wsdl_file);
                break;
            case 'msg': {
                my list names;
                if (params[0] && params[0] != '-') {
                    names = params[0].split(',');
                } else {
                    names = keys ws.opmap;
                }
                info(sprintf("%s(%y,%y,%y,%y,%y)\n", operation, names, params[1], opt.format, opt.comment, opt.choices));
                foreach my string name in (names) {
                    if (!ws.opmap{name}) {
                        throw "WSDL-TOOL-ERROR", sprintf("Value %y is not in the list of operations %y\n", name, keys ws.opmap);
                    }
                    my WSOperation op = ws.opmap{name};
                    foreach my int request in ((MSG_REQUEST, MSG_RESPONSE)) {
                        if ((request & params[1]) != 0 && ((request==MSG_REQUEST) ? op.input : op.output)) {
                            my hash msg = wh.getMessage((request==MSG_REQUEST) ? op.input : op.output, opt.comment && opt.format != 'xml', False, !opt.choices || opt.format == 'xml');
                            switch (opt.format) {
                            case 'xml':
                                if (opt.comment) {
                                    info("Comment options is ignored\n");
                                }
                                if (opt.choices) {
                                    info("Multiple choices options is ignored\n");
                                }
                                info("Only the first choice is considered\n");
                                output.print(request == MSG_REQUEST ?
                                    op.serializeRequest(msg, NOTHING, NOTHING, NOTHING, NOTHING, XGF_ADD_FORMATTING).body :
                                    op.serializeResponse(msg, NOTHING, NOTHING, NOTHING, NOTHING, XGF_ADD_FORMATTING).body
                                );
                                break;
                            case 'qore':
                                output.print(qorize(msg, name, True));
                                break;
                            case 'human':
                                output.printf("%s: %N\n", name, msg);
                                break;
                            }
                            output.print("\n\n");
                        }
                    }
                }
            }
        }
    }

    private help() {
        stderr.printf(
            "WSDL tools, ver 0.1\n"
            "usage: %s [options] [-o <output>] [-f <format>] <wsdl> [<cmd> [params]]\n"
            "  options\n"
            "  -v     verbose\n"
            "  -h     help\n"
            "  wsdl   WSDL file or URL. If '-' specified then expected at STDIN\n"
            "  output unless file specified then goes to STDOUT\n"
            "\n"
            "Commands:\n"
            "  report\n"
            "    report supported services, operations, etc.\n"
            "\n"
            "  msg [<operation> [both|request|response]]\n"
            "    print request or response message related to operation\n"
            "    operation    unless specified (or '-') print all operations\n"
            "    format:\n"
            "      human  human-readable, default\n"
            "      qore   Qore source file\n"
            "      xml    XML SOAP message\n"
            "    options:\n"
            "      -c     output comments\n"
            "      -m     output multiple choices\n"
            "\n"
            "  dump <what>\n"
            "    dump specified information\n"
            "    What:\n"
            "      wsdl   print WSDL on output\n"
            "      ws     print WebService on output\n"
            "\n"
            "\n"
            "Example:\n"
            "  %s MaterialMovementInterface_v13.wsdl msg exportMaterialMovementData request -f qore\n"
            "\n"
        ,
            get_script_name(),
            get_script_name(),
        );
        exit(1);
    }

    private info(string s) { # , any *p1, any *p2, any *p3, any *p4, any *p5) {  # TODO: elipsis params and vprintf()
        if (verbose) {
            stderr.print(s);
            #stderr.vprintf(s, p1, p2, p3, p4, p5);
        }
    }
}
