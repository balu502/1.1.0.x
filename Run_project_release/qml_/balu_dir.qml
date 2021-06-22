//import QtQuick 2.4          
import QtQuick 2.0
import QtQuick.Controls 1.0
import Qt.labs.folderlistmodel 2.1

//import WSDir 1.0

Rectangle
{
    id:     root
//    color:  "green"

    width:   10
    height:  10

/*    width:  200
    height: 200
*/
//    visible: false
//    opacity: 0.5

    //property string sourceDIR:  "file:///exchange"
    property string sourceDIR:  "exchange"
    property string pathList:	sourceDIR+"/protocols"
    property string pathSave:	sourceDIR+"/reply"

    property var arrProts:	Object;
    property var idxProts:	Object;
    property var arrReply:	Object;

    property bool firstWrite:	true;

    Component.onCompleted: {
	Web.connection_Server = true;
//       web.connection_Server = true;
//       lsTimer.running       = true;
//       var xsltFile= "import_lis_file.xsl"
//       var xmlFile = "1164.xml"
//       console.log( xtItem.transfomXSLT(xmlFile, xsltFile) )

	arrProts = {};
	idxProts = {};
	arrReply = {};
   }

    FolderListModel {
	id: folderReq
        showDirs: false
        folder: pathList
        nameFilters: ["*.rt"]

	onCountChanged: loadProtocols();
    }

    FolderListModel {
	id: folderDone
        showDirs: false
        folder: pathSave
        nameFilters: ["*.rt"]

	property bool isInitial: true;
	onCountChanged: notifyReply();
    }

/*
   Item {

       id: fioItem

       WrapperAx{ id: sax; }
       WrapperAx{ id: fax; }

       function writeFile( fpath, strText ){

            fax.setControl("Scripting.FileSystemObject")

            //fpath = "C:\\Test.txt"

            if( fax.isInvalid )
                return false;

            fax.querySubObject(sax, "CreateTextFile(QString)", fpath);

            if( sax.isInvalid )
                return false;

            sax.dynamicCall("WriteLine(QString)", strText)
            sax.dynamicCall("Close()")

            return true;
       }

   }
*/
/*
    Item {

        id: xtItem

        WrapperAx{ id: wax; }
        WrapperAx{ id: dax; }

        function transfomXSLT( xmlFile, xsltFile ){

            wax.setControl("Msxml2.DOMDocument.5.0")
            dax.setControl("Msxml2.DOMDocument.5.0")

            wax.setProperty("AllowDocumentFunction", true);
            dax.setProperty("AllowDocumentFunction", true);

            wax.setProperty( "async", false )
            dax.setProperty( "async", false )

            dax.setProperty( "validateOnParse", false )

            wax.dynamicCall( "load(QVariant)",  xmlFile )
            dax.dynamicCall( "load(QVariant)", xsltFile )

            if (!wax.getProperty( "parsed" )
             || !dax.getProperty( "parsed" )){
                console.warn("Cannot parse xml document(s)")
                return "";
            }

            return wax.dynamicCall( "transformNode(IDispatch*)", dax.asVariant() )
        }


/*
        function transfomXSLT( xmlFile, xsltFile ){

           wax.setControl("Msxml2.FreeThreadedDOMDocument.6.0")
           dax.setControl("Msxml2.FreeThreadedDOMDocument.6.0")
           tax.setControl("Msxml2.XSLTemplate.6.0")

           if(!wax.isInvalid ){

               wax.setProperty( "async", false )
               dax.setProperty( "async", false )

               wax.setProperty("AllowDocumentFunction", true);
               dax.setProperty("AllowDocumentFunction", true);

               wax.setProperty("resolveExternals", true);
               dax.setProperty("resolveExternals", true);

        //           wax.dynamicCall( "loadXML(QString)", strXML )
               wax.dynamicCall( "load(QVariant)",  xmlFile )
               dax.dynamicCall( "load(QVariant)", xsltFile )

               if (!wax.getProperty( "parsed" )
                || !dax.getProperty( "parsed" )){
                   console.warn("Cannot parse xml document")
                   return "";
               }


               tax.setProperty( "AllowDocumentFunction",true)
               tax.setProperty( "resolveExternals",     true)
               tax.setProperty( "stylesheet", dax.asVariant())
               tax.querySubObject( pax,  "createProcessor()" )

               pax.setProperty( "AllowDocumentFunction",true)
               pax.setProperty( "resolveExternals",     true)
               pax.setProperty( "input",     wax.asVariant())

               //pax.dynamicCall( "addParameter(QString)", "par_val_1");

               console.log( "AllowDocumentFunction " + wax.getProperty("AllowDocumentFunction"));
               console.log( "AllowDocumentFunction " + dax.getProperty("AllowDocumentFunction"));
               console.log( "AllowDocumentFunction " + tax.getProperty("AllowDocumentFunction"));
               console.log( "AllowDocumentFunction " + pax.getProperty("AllowDocumentFunction"));


               pax.dynamicCall( "transform()" )

               return   pax.getProperty( "output" );
           }

        }
    /* /
    }
*/

    states: [
	    State { name: "undef";   PropertyChanges { target: control_status; color: "gray"; } },
	    State { name: "conn";    PropertyChanges { target: control_status; color: "lightgreen";  } },
	    State { name: "broken";  PropertyChanges { target: control_status; color: "red";    } }
    ]

    state: "undef"

    transitions: [
	Transition {
        	PropertyAnimation { 
			target: control_status; 
			properties: "color"; 
			duration: 100; loops: 2 } 
	}]


    Rectangle
    {
        id: control_status
        x:  5;	y:  5;
        color:  "gray"
        width:   10
        height:  10
    }


    function subscribe()
    {
       return "";
    }


    function getListProtocols()
    {
	var sjoined = [];

	for ( var ii in arrProts ) 
		sjoined.push( ii );

//	web.get_ListProtocols( sjoined.join(':'), ':' );
	Web.get_ListProtocols( sjoined.join(':'), ':' );

	state = "conn";
    }


    function loadProtocols(){

	state = "undef";

	console.log( "------------------------" );
	console.log( "root:"  +folderReq.rootFolder );
	console.log( "count:" +folderReq.count );

	var sjoined = [];
	var pn = "";

	for (var ii= 0; ii < folderReq.count; ++ii){
		pn = folderReq.get(ii, "fileName");

		loadProtocolHeader( pn, ii+1 == folderReq.count );
	}

	state = "conn";
    }

    function hashCode(s){
	return s.split("").reduce(function(a,b){a=((a<<5)-a)+b.charCodeAt(0);return a&a},0); 
    }

    function nodeText(n){
	return ( n.childNodes.length ) ? n.childNodes[0].nodeValue : "";
    }

    function loadProtocolHeader( fileName, isLast )
    {
	console.log( "loadProtocolHeader " + fileName );

	if ( ( arrProts[ fileName ] != "" )
	&&   ( arrProts[ fileName ] != undefined )
	){
		isLast && getListProtocols();
		return;
	}

	var doc      = new XMLHttpRequest();
	var addrScr  = pathList +"/"+ fileName;

	state = "undef";

	doc.onreadystatechange = function() {

           if (doc.readyState === XMLHttpRequest.DONE) {

		state = "conn";
		arrProts[ fileName ] = "";

		if ( doc.responseXML === null )
                   	console.log("Unable to get protocol");
		else{

			var iarr= [ "", "", "" ];
            		var a = doc.responseXML.documentElement;
			var f3= 0;

			for (var ii = 0; ii < a.childNodes.length; ++ii) {

				if ( a.childNodes[ii].nodeName == "regNumber" )
					{iarr[0] = nodeText( a.childNodes[ii] ); f3++}
				else
				if ( a.childNodes[ii].nodeName == "barcode" )
					{iarr[1] = nodeText( a.childNodes[ii] ); f3++}
				else
				if ( a.childNodes[ii].nodeName == "name" )
					{iarr[2] = nodeText( a.childNodes[ii] ); f3++}
				else
				if (f3 == 3) break;
            		}


			if (iarr[2] == "") iarr[2] = fileName;
			if (iarr[1] == "") iarr[1] = "i"+hashCode( fileName );
			if (iarr[0] == "") iarr[0] = iarr[1];

			arrProts[ fileName ] = iarr.join(";");
			idxProts[ iarr[0]  ] = fileName;

/*			console.log( arrProts[ fileName ] );
*/		}

		isLast && getListProtocols();
           }
	}

	doc.open("GET", addrScr);
	doc.send()
    }


    function getWebProtocol( id_protocol )
    {
	console.log( "getWebProtocol " + id_protocol );

	if ( undefined == idxProts[ id_protocol ] )
		return;

	var doc      = new XMLHttpRequest();
	var addrScr  = pathList+"/"+idxProts[ id_protocol ];

	state = "undef";

	doc.onreadystatechange = function() {

           if (doc.readyState === XMLHttpRequest.DONE) {

	       state = "conn";

               if (doc.responseXML === null){
                   console.log("Unable to get responce");
                   return;
               }

                var str = doc.responseText;
                Web.get_Protocol( str );
//		web.get_Protocol( str );

//              transfomXSLT( str )
           }
	}

	doc.open("GET", addrScr);
	doc.send();
    }


    function notifyReply(){

	if ( folderDone.isInitial ){
		var pn, fsz;
		for (var ii= 0; ii < folderDone.count; ++ii){
			pn = folderDone.get(ii, "fileName");
			fsz= folderDone.get(ii, "fileSize");
			arrReply[ pn ] = fsz;
		}

		folderDone.isInitial = false;
		return;
	}

	for (var ii= 0; ii < folderDone.count; ++ii){
		var pn, fsz;
		pn = folderDone.get(ii, "fileName");
	
		if ( arrReply[ pn ] == undefined ){
			fsz= folderDone.get(ii, "fileSize");
			arrReply[ pn ] = fsz;

//			web.send_Message( "Reply complete:"+ pn );
			Web.send_Message( "Reply complete:"+ pn );
		}
	}
    }


    function send_ProtocolToWeb( id_protocol, data_protocol )
    {
	console.log( "send_ProtocolToWeb " + id_protocol );

	if ( undefined == idxProts[ id_protocol ] ){
		console.log( id_protocol + " is undefined index for protocol" );
		return;
	}

	var doc      = new XMLHttpRequest();
	var fName    = idxProts[ id_protocol ];
	var addrScr  = pathSave +"/"+ fName;

	state = "undef";

	doc.onreadystatechange = function() {
		state = "conn";
		arrReply[ fName ] = undefined;
		notifyReply();
	}


	console.log( "Writing file " + fName + "..." );

	doc.open("PUT", addrScr, true);
	doc.send( data_protocol );

	if ( firstWrite ){
		doc.open("PUT", addrScr, true);
		doc.send( data_protocol );
		firstWrite = false;
	}

//	return doc.status;
    }


    function send_StateProtocolToWeb( id_protocol, state_protocol )
    {
	console.log( "send_StateProtocolToWeb " + id_protocol );
    }


   function set_StateSocket(state)
   {
//       socket.active = state;
       lsTimer.running = state;
   }

   // TODO: comment Item 'web' declaration and replace 'web' to 'Web' inside this script
   //
/*
   Item{
       id: web
       property bool connection_Server: false
       function get_ListProtocols(str,sep){ console.log("get_ListProtocols") }
       function get_Protocol(str){ console.log("get_Protocol "+ str.substring(0, 200) ) }
       function send_Message(str){ console.log("send_Message "+ str.substring(0, 200) ) }
   }
*/

   Item{
       id: iClient
       property string  md5List: "-1"

       Timer{
           id:lsTimer
           interval: 5000; running: false; repeat: true
/*         interval: 5000; running: true; repeat: true */
           triggeredOnStart:true;	

           onTriggered: { loadProtocols(); }
/*           onTriggered: { getListProtocols(); }*/
       }
   }

/*
   Button
   {
        id: button
        anchors.centerIn: parent
        width:  parent.width/2;
        height: parent.height/2;

        onClicked:
        {
		send_ProtocolToWeb( "0", "pdapdatata" );
//		getWebProtocol( "0" );
//		loadProtocols();

//            lsTimer.running = !lsTimer.running;
//            button.text = lsTimer.running? "Disconnect": "Connect.."
        }

        Component.onCompleted:
        {
            button.text = lsTimer.running? "Disconnect": "Connect.."
        }
   }
*/
}


