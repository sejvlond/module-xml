# -*- mode: qore; indent-tabs-mode: nil -*-
#! @file SoapClient.qc SOAP Client implementation based on the WSDL classes

# a minimal SOAP client using WSDL, XSD, SOAP support implemented in WSDL.qc
# by David Nichols

# to use this class: %include WSDL.qc
#                    %include SoapClient.qc
#                    %include MultiPartMessage.qc
#
# the constructor takes named arguments in the form of a hash
# vaild arguments are:
# required keys: one of: "wsdl" or "wsdl_file"
#                                  : a string defining the WSDL or the URL of
#                                    the WSDL
# optional keys:         "service" : the name of the "portType" to use (if
#                                    more than 1 portType is defined in the 
#                                    WSDL then this key is mandatory
#                        "url"     : to override the URL defined in the WSDL
#                        "headers" : to override any HTTP headers sent in 
#                                    outgoing messages
#                        "event_queue" : to set an event queue on the
#                                        HTTPClient
#
# also the following keys can be set to set HTTP options:
#   "connect_timeout", "http_version", "max_redirects", "proxy", "timeout"
#
# create messages by setting up a Qore data structure corresponding to the SOAP
# message.  Exceptions will be thrown if either the outgoing or the response
# message do not corespond to the WSDL.   The exceptions should be fairly verbose
# to allow you to quickly correct any mistakes.
#
# currently the WSDL implementation is fairly basic so any messages using
# unimplemented features of SOAP or XSD will fail.
#
# example: (make sure the files are in the same directory or in the
#           QORE_INCLUDE_DIR path)
#
# %include WSDL.qc
# %include SoapClient.qc
# my SoapClient $sc = new SoapClient(("wsdl" : "http://soap.server.org:8080/my-service?wsdl"));
# my any $result = $sc.call("SubmitDocument", $msg);

# we need qore 0.7.3 or later for parseXMLAsData
# qore 0.8 for hard typing
%requires qore >= 0.8

#! SOAP client class implementation, publically inherits qore's HTTPClient class
class SoapClient inherits HTTPClient {
    #! version of the implementation of this class
    const Version = "0.2.2";
    #! default HTTP headers
    const Headers = ("Accept":"application/soap+xml,text/xml", "User-Agent":("Qore Soap Client " + SoapClient::Version));
    #! option keys passed to the HTTPClient constructor
    const HTTPOptions = ( "connect_timeout", "http_version", "max_redirects", "proxy", "timeout" );

    private {
        WebService $.wsdl;  # web service definition
        string $.svc;   # service name
    }

    public {
        #! target URL
        string $.url;
        #! HTTP headers to use
        hash $.headers = Headers;
    }

    #! creates the object based on a %WSDL which is parsed to a WSDL::WebService object which provides the basis for all communication with this object
    /** one of either the \c wsdl or \c wsdl_file keys is required in the hash given to the constructor or an exception will be thrown
        @param $h valid option keys:\n- \c wsdl: the URL of the web service or a WSDL::WebService object itself\n- \c wsdl_file: a path to use to load the %WSDL and create the WSDL::WebService object\n- \c url: override the target URL given in the %WSDL\n- also all options from SoapClient::HTTPOptions, which are passed to the HTTPClient constructor
      */
    constructor(hash $h) : HTTPClient($h{HTTPOptions}) {
	if (exists $h.wsdl_file && exists $h.wsdl)
	    throw "SOAP-CLIENT-ERROR", "only one of 'wsdl' or 'wsdl_file' keys can be given; both were passed";

	if (exists $h.event_queue)
	    $.setEventQueue($h.event_queue);

	my any $wsdl;
	# get web service definition
	if (exists $h.wsdl_file)
	    $wsdl = WSDLLib::getWSDL($h.wsdl_file, $self, $h.headers);
	else if (exists $h.wsdl)
	    $wsdl = WSDLLib::getWSDL($h.wsdl, $self, $h.headers);
	else
	    throw "SOAP-CLIENT-ERROR", "neither one of required 'wsdl' or 'wsdl_file' keys is present in the hash argument to SoapClient::constructor()";

	if (!exists $wsdl)
	    throw "SOAP-CLIENT-ERROR", "missing wsdl in SoapClient::constructor()";

	$.wsdl = $wsdl instanceof WebService ? $wsdl : new WebService($wsdl, ("http_client" : $self, "http_headers" : $h.headers) + $h.wsdl_opt);

	# set service
	# get list of services in this wsdl
	my any $svcs = keys $.wsdl.portType;
	if (elements $svcs > 1 && !exists $h.service)
	    throw "SOAP-CLIENT-ERROR", sprintf("no 'service' key passed in the option hash argument to SoapClient::constructor() (WSDL defines the following services: %n)", $svcs);
	
	if (exists $h.service) {
	    if (!inlist($h.service, $svcs))
		throw "SOAP-CLIENT-ERROR", sprintf("service %n is not defined by this WSDL (valid services: %n)", $h.service, $svcs);
	    $.svc = $h.service;
	}
	else
	    $.svc = $svcs[0];
	
	if (exists $h.url)
	    $.url = $h.url;
	else {
	    if (elements $.wsdl.services.port > 1)
		throw "SOAP-CLIENT-ERROR", sprintf("don't know how to handle more than one port in a WSDL (this WSDL has %n)", keys $.wsdl.services.port);
	    my string $port = (keys $.wsdl.services.port)[0];

	    $.url = $.wsdl.services.port.$port.address;
	}

	$.headers += $h.headers;
	# setup default headers
	if ($.wsdl.isSoap12())
	    $.headers += ("Content-Type":"application/soap+xml");
	else
	    $.headers += ("Content-Type":"text/xml");

	# set URL
	$.setURL($.url);
	#printf("DEBUG: set url to %n\n", $.url);
    }

    #! returns a hash representing the serialized SOAP request for a given WSOperation
    /** the returned hash can be passed to makeXMLString() to make the actual SOAP message
        @param $operation the SOAP operation to use to serialize the request; if the operation is not known to the underlying WebService class, an exception will be thrown
        @param $h the operation parameter(s)
        @param $op a reference to return the WSOperation object found
      */
    hash getMsg(string $operation, any $h, reference $op) {
	$op = $.wsdl.portType.$.svc.operations.$operation;
	if (!exists $op)
	    throw "SOAP-CLIENT-ERROR", sprintf("operation %n does not exist (operations defined by service %n: %n)", 
					       $operation, $.svc, keys $.wsdl.portType.$.svc.operations);

	return $op.serializeRequest($h, $.headers);
    }

    #! makes a server call with the given operation and arguments and returns the deserialized result
    /** @param $operation the operation name for the SOAP call
        @param $h the operation parameter(s)
        @param $info an optional reference to return technical information about the SOAP call (raw message info and headers)
        @return the deserialized result of the SOAP call to the SOAP server
      */
    any call(string $operation, any $h, any $info) {
	my WSOperation $op;
	my hash $msg = $.getMsg($operation, $h, \$op);

	# we have to write the request key after the HTTPClient::post() call
        on_exit $info.request = $msg;

	$info.response = $.send($msg.body, "POST", $.url, $.headers + $msg.hdr, True, \$info);

	my hash $xmldata = WSDLLib::parseSOAPMessage($info.response, $info.response.body);
	
	#printf("DEBUG ans=%n\n", $info.response);
	return $op.deserializeResponse($xmldata);
    }

    #! uses SoapClient::call() to transparently serialize the argument and make a call to the given operation and return the deserialized results
    /** @param $op the operation name, which is the method name passed to methodGate()
        @return the deserialized result of the SOAP call to the SOAP server
      */
    any methodGate(string $op) {
        return $.call($op, $argv[0]);
    }
}
