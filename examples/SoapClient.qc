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
# qore 0.8.2 for hard typing
# qore 0.8.4 for pseudo-methods
%requires qore >= 0.8.4

#! SOAP client class implementation, publically inherits qore's HTTPClient class
class SoapClient inherits HTTPClient {
    #! version of the implementation of this class
    const Version = "0.2.2";
    #! default HTTP headers
    const Headers = ("Accept": (MimeTypeSoapXml + "," + MimeTypeXml + "," + MimeTypeXmlApp), "User-Agent": ("Qore Soap Client " + SoapClient::Version));
    #! option keys passed to the HTTPClient constructor
    const HTTPOptions = ( "connect_timeout", "http_version", "max_redirects", "proxy", "timeout" );

    private {
        WebService $.wsdl;  # web service definition
        string $.portType;   # service name
    }

    public {
        #! target URL
        string $.url;
        #! HTTP headers to use
        hash $.headers = Headers;
    }

    #! creates the object based on a %WSDL which is parsed to a WSDL::WebService object which provides the basis for all communication with this object
    /** one of either the \c wsdl or \c wsdl_file keys is required in the hash given to the constructor or an exception will be thrown
        @param $h valid option keys:
	- \c wsdl: the URL of the web service or a WSDL::WebService object itself
	- \c wsdl_file: a path to use to load the %WSDL and create the WSDL::WebService object
	- \c url: override the target URL given in the %WSDL
	- [\c portType]: in case multiple portType entries are found in the WSDL, give the one to be used here
	- [\c port]: in case multiple port entries are found in the WSDL, give the one to be used here
	- also all options from SoapClient::HTTPOptions, which are passed to the HTTPClient constructor
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
	my list $portTypes = keys $.wsdl.portType;
	if (elements $portTypes > 1 && !exists $h.portType)
	    throw "SOAP-CLIENT-ERROR", sprintf("no 'portType' key passed in the option hash argument to SoapClient::constructor() (WSDL defines the following services: %n)", $portTypes);
	
	if (exists $h.portType) {
	    if (!inlist($h.portType, $portTypes))
		throw "SOAP-CLIENT-ERROR", sprintf("portType %n is not defined by this WSDL (valid portTypes: %y)", $h.portType, $portTypes);
	    $.portType = $h.portType;
	}
	else
	    $.portType = $portTypes[0];
	
	if (exists $h.url)
	    $.url = $h.url;
	else {
	    my string $port;
	    if (elements $.wsdl.services.port > 1) {
		if (exists $h.binding) {
		    foreach my string $p in (keys $.wsdl.services.port) {
			#printf("b=%y == %y\n", $.wsdl.services.port.$p.binding.val, $h.binding);
			if ($.wsdl.services.port.$p.binding.val === $h.binding) {
			    $port = $p;
			    break;
			}
		    }
		    if (!exists $port)
			throw "SOAP-CLIENT-ERROR", sprintf("cannot find any port with binding: %y (valid ports: %y)", $h.binding, keys $.wsdl.services.port);
		}
		else if (exists $h.port) {
		    if (!exists $.wsdl.services.port)
			throw "SOAP-CLIENT-ERROR", sprintf("cannot find any port %y (valid ports: %y)", $h.port, keys $.wsdl.services.port);
		    $port = $h.port;
		}
		else
		    throw "SOAP-CLIENT-ERROR", sprintf("don't know how to handle more than one port in a WSDL (this WSDL has %n)", keys $.wsdl.services.port);
	    }
	    else
		$port = ($.wsdl.services.port).firstKey();

	    $.url = $.wsdl.services.port.$port.address;
	}

	$.headers += $h.headers;
	# setup default headers
	if ($.wsdl.isSoap12())
	    $.headers += ("Content-Type": MimeTypeSoapXml);
	else
	    $.headers += ("Content-Type": MimeTypeXml);

	# set URL
	$.setURL($.url);
	#printf("DEBUG: set url to %n\n", $.url);
    }

    #! returns a hash representing the serialized SOAP request for a given WSOperation
    /** the returned hash can be passed to makeXMLString() to make the actual SOAP message
        @param $operation the SOAP operation to use to serialize the request; if the operation is not known to the underlying WebService class, an exception will be thrown
        @param $h the operation parameter(s)
        @param $header data structure for the SOAP header, if required by the message
        @param $op a reference to return the WSOperation object found
      */
    hash getMsg(string $operation, any $h, any $header, reference $op) {
	if (!exists $.wsdl.portType.$.portType.operations.$operation)
	    throw "SOAP-CLIENT-ERROR", sprintf("operation %n does not exist (operations defined by service %n: %y)", 
					       $operation, $.portType, keys $.wsdl.portType.$.portType.operations);

	$op = $.wsdl.portType.$.portType.operations.$operation;
	my hash $msg = $op.serializeRequest($h, $header, $.getEncoding());
	if ($msg.hdr."Content-Type" !~ /charset=/i)
	    $msg.hdr."Content-Type" += ";charset=" + $.getEncoding();
	return $msg;
    }

    #! makes a server call with the given operation and arguments and returns the deserialized result
    /** @param $operation the operation name for the SOAP call
        @param $h the operation parameter(s)
	@param $header optional soap headers (if required by the operation)
        @return the deserialized result of the SOAP call to the SOAP server
      */
    any call(string $operation, any $h, *hash $header) {
	my WSOperation $op;
	my hash $msg = $.getMsg($operation, $h, $header, \$op);
	my hash $response = $.send($msg.body, "POST", $.url, $.headers + $msg.hdr, True);
	my hash $xmldata = WSDLLib::parseSOAPMessage($response);	
	return $op.deserializeResponse($xmldata);
    }

    #! makes a server call with the given operation and arguments and returns the deserialized result
    /** @param $operation the operation name for the SOAP call
        @param $h the operation parameter(s)
	@param $info an optional reference to return a hash of technical information about the SOAP call (raw message info and headers)
        @return the deserialized result of the SOAP call to the SOAP server
      */
    any call(string $operation, any $h, reference $info) {
	my WSOperation $op;
	my hash $msg = $.getMsg($operation, $h, NOTHING, \$op);
	# we have to write the request key after the HTTPClient::post() call
        on_exit $info.request.body = $msg.body;
	$info.request.headers = $.headers + $msg.hdr;
	$info.response = $.send($msg.body, "POST", $.url, $info.request.headers, True, \$info);
	my hash $xmldata = WSDLLib::parseSOAPMessage($info.response);
	#printf("DEBUG ans=%n\n", $info.response);
	return $op.deserializeResponse($xmldata);
    }

    #! makes a server call with the given operation and arguments and returns the deserialized result with an output argument giving technical information about the call
    /** @param $info an optional reference to return a hash of technical information about the SOAP call (raw message info and headers)
	@param $operation the operation name for the SOAP call
        @param $h the operation parameter(s)
	@param $header optional soap headers (if required by the operation)
        @return the deserialized result of the SOAP call to the SOAP server
      */
    any call(reference $info, string $operation, any $h, any $header) {
	my WSOperation $op;
	my hash $msg = $.getMsg($operation, $h, $header, \$op);
	my hash $hdr = $.headers + $msg.hdr;

	# we have to write the request key after the HTTPClient::post() call
        on_exit $info.request = ("body": $msg.body, "headers": $hdr);

	$info.response = $.send($msg.body, "POST", $.url, $hdr, True, \$info);

	my hash $xmldata = WSDLLib::parseSOAPMessage($info.response);
	
	#printf("DEBUG ans=%n\n", $info.response);
	return $op.deserializeResponse($xmldata);
    }

    #! uses SoapClient::call() to transparently serialize the argument and make a call to the given operation and return the deserialized results
    /** @param $op the operation name, which is the method name passed to methodGate()
        @param $arg a list or arguments or a single argument (or NOTHING) for the operation
        @return the deserialized result of the SOAP call to the SOAP server
      */
    any methodGate(string $op, any $arg) {
        return $.call($op, $arg);
    }
}
