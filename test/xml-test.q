#!/usr/bin/env qore 
# -*- mode: qore; indent-tabs-mode: nil -*-

# require the xml module
%requires xml

# require all global variables to be declared with "our"
%require-our
# enable all warnings
%enable-all-warnings
# child programs do not inherit parent's restrictions
%no-child-restrictions
# require types to be declared
%require-types

# make sure we have the right version of qore
%requires qore >= 0.8.1

our (hash $o, int $errors);
our hash $thash;

sub usage() {
    printf(
"usage: %s [options] <iterations>
  -h,--help         shows this help text
  -t,--threads=ARG  runs tests in ARG threads
  -v,--verbose=ARG  sets verbosity level to ARG
", 
	   get_script_name());
    exit(1);
}

const opts = 
    ( "verbose" : "verbose,v:i+",
      "help"    : "help,h",
      "threads" : "threads,t=i" );

sub parse_command_line() {
    my GetOpt $g(opts);
    $o = $g.parse(\$ARGV);
    if (exists $o."_ERRORS_") {
        printf("%s\n", $o."_ERRORS_"[0]);
        exit(1);
    }
    if ($o.help)
	usage();

    $o.iters = shift $ARGV;
    if (elements $ARGV) {
	printf("error, excess arguments on command-line\n");
	usage();
    }

    if (!$o.iters)
	$o.iters = 1;
    if (!$o.threads)
	$o.threads = 1;
}

sub test_value(any $v1, any $v2, string $msg) {
    if ($v1 === $v2) {
	if ($o.verbose)
	    printf("OK: %s test\n", $msg);
    }
    else {
	printf("ERROR: %s test failed! (%N != %N)\n", $msg, $v1, $v2);
	#printf("%s%s", dbg_node_info($v1), dbg_node_info($v2));
	$errors++;
    }
    $thash.$msg = True;
}

const xsd = '<?xml version="1.0" encoding="utf-8"?>
<xsd:schema targetNamespace="http://qoretechnologies.com/test/namespace" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
  <xsd:element name="TestElement">
    <xsd:complexType>
      <xsd:simpleContent>
        <xsd:extension base="xsd:string"/>
      </xsd:simpleContent>
    </xsd:complexType>
  </xsd:element>
</xsd:schema>
';

sub xml_tests() {
    my hash $o = ( "test" : 1, 
		   "gee" : "philly", 
		   "marguile" : 1.0392,
		   "list" : (1, 2, 3, ( "four" : 4 ), 5),
		   "hash" : ( "howdy" : 123, "partner" : 456 ),
		   "list^1" : "test",
		   "bool" : True,
		   "time" : now(),
		   "bool^1" : False,
		   "key"  : "this & that" );
    my hash $mo = ( "o" : $o );
    my string $str = makeXMLString("o", $o);
    test_value($mo == parseXML($str), True, "first parseXML()");
    $str = makeFormattedXMLString("o", $o);
    test_value($mo == parseXML($str), True, "second parseXML()");
    my list $params = (1, True, "string", NOTHING, $o);
    $str = makeXMLRPCCallStringArgs("test.method", $params);
    my hash $result = ( "methodName" : "test.method", "params" : $params );    
    test_value(parseXMLRPCCall($str), $result, "makeXMLRPCCallStringArgs() and parseXMLRPCCall()");
    $str = makeFormattedXMLRPCCallStringArgs("test.method", $params);

    test_value(parseXMLRPCCall($str), $result, "makeFormattedXMLRPCCallStringArgs() and parseXMLRPCCall()");
    $str = makeXMLRPCCallString("test.method", True, $o);
    $result = ( "methodName" : "test.method","params" : (True, $o) );
    test_value(parseXMLRPCCall($str), $result, "makeXMLRPCCallString() and parseXMLRPCCall()");
    $str = makeFormattedXMLRPCCallString("test.method", True, $o);
    test_value(parseXMLRPCCall($str), $result, "makeFormattedXMLRPCCallString() and parseXMLRPCCall()");
    $str = makeXMLRPCResponseString($o);
    test_value(parseXMLRPCResponse($str), ( "params" : $o ), "first makeXMLRPCResponse() and parseXMLRPCResponse()");
    $str = makeFormattedXMLRPCResponseString($o);
    test_value(parseXMLRPCResponse($str), ( "params" : $o ), "first makeFormattedXMLRPCResponse() and parseXMLRPCResponse()");
    $str = makeXMLRPCFaultResponseString(100, "error");
    my hash $fr = ( "fault" : ( "faultCode" : 100, "faultString" : "error" ) );
    test_value(parseXMLRPCResponse($str), $fr, "second makeXMLRPCResponse() and parseXMLRPCResponse()");
    $str = makeFormattedXMLRPCFaultResponseString(100, "error");
    test_value(parseXMLRPCResponse($str), $fr, "second makeXMLRPCResponse() and parseXMLRPCResponse()");
    $o = ( "xml" : ($o + ( "^cdata^" : "this string contains special characters &<> etc" )) );
    test_value($o == parseXML(makeXMLString($o)), True, "xml serialization with cdata");

    if (Option::HAVE_PARSEXMLWITHSCHEMA) {
        $o = ( "ns:TestElement" : ( "^attributes^" : ( "xmlns:ns" : "http://qoretechnologies.com/test/namespace" ), "^value^" : "testing" ) );

        test_value(parseXMLWithSchema(makeXMLString($o), xsd), $o, "parseXMLWithSchema()");
    }

    $str = makeXMLString($mo);
    my XmlDoc $xd = new XmlDoc($str);
    test_value($xd.toQore() == $mo, True, "XmlDoc::constructor(<string>), XmlDoc::toQore()");
    test_value(parseXML($xd.toString()) == $mo, True, "XmlDoc::toString()");
    my XmlNode $n = $xd.evalXPath("//list[2]")[0];
    test_value($n.getContent(), "2", "XmlDoc::evalXPath()");
    test_value($n.getElementTypeName(), "XML_ELEMENT_NODE", "XmlNode::getElementTypeName()");
    $n = $xd.getRootElement().firstElementChild();
    test_value($n.getName(), "test", "XmlDoc::geRootElement(), XmlNode::firstElementChild(), XmlNode::getName()");
    $n = $xd.getRootElement().lastElementChild();
    test_value($n.getName(), "key", "XmlNode::lastElementChild()");
    test_value($n.previousElementSibling().getName(), "bool", "XmlNode::previousElementSibling()");
    test_value($xd.getRootElement().childElementCount(), 14, "XmlNode::childElementCount()");

    $xd = new XmlDoc($mo);
    test_value($xd.toQore() == $mo, True, "XmlDoc::constructor(<hash>), XmlDoc::toQore()");

    my XmlReader $xr = new XmlReader($xd);
    # move to first element
    $xr.read();
    test_value($xr.nodeType(), Xml::XML_NODE_TYPE_ELEMENT, "XmlReader::read(), XmlReader::Type()");
    test_value($xr.toQore() == $mo.o, True, "XmlReader::toQoreData()");
}

sub do_tests() {
    on_exit $counter.dec();
    try {
	for (my int $i = 0; $i < $o.iters; $i++) {
	    if ($o.verbose)
		printf("TID %d: iteration %d\n", gettid(), $i);
	    xml_tests();
	}
    }
    catch () {
	++$errors;
	rethrow;	
    }
}

sub main() {
    parse_command_line();
    printf("QORE v%s XML Module v%s Test Script (%d thread%s, %d iteration%s per thread)\n", 
           Qore::VersionString, getModuleHash().xml.version,
	   $o.threads, $o.threads == 1 ? "" : "s", $o.iters, $o.iters == 1 ? "" : "s");

    our Counter $counter = new Counter();
    while ($o.threads--) {
	$counter.inc();
	background do_tests();
    }

    $counter.waitForZero();

    my int $ntests = elements $thash;
    printf("%d error%s encountered in %d test%s.\n",
	   $errors, $errors == 1 ? "" : "s", 
	   $ntests, $ntests == 1 ? "" : "s");
}

main();
