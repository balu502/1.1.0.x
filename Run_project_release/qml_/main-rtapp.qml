import QtQuick 2.4
//import QtQuick 2.0
//import QtQuick.Controls 1.0
//import WSDir 1.0

Rectangle
{
    id:     root
    color:  "white"
    width:  200
    height: 200

    property string accountGUID: "AccountGUID=7c890b798cd940999a4bbd529aa1e6e0"
    property string hostURL:	 "http://10.1.1.48?" + accountGUID

    property string addrList:	"http://10.1.1.48/GetProtocolListXML?"   +accountGUID
    property string addrProt:	"http://10.1.1.48/GetProtocolXML?"       +accountGUID
    property string addrSave:	"http://10.1.1.48/SetProtocolResultsXML"

    function set_SocketUrl(url)
    {
        print("set_SocketUrl(url): \""+ url +"\"");

        if ( String(url).trim().length ){

            var splUrl = String(url).split("?");
            var guidURL=  (splUrl.length>1) ? splUrl[ 1 ] : "" ;

            hostURL = url;
            addrList= splUrl[0] +"/GetProtocolListXML?"     +guidURL;
            addrProt= splUrl[0] +"/GetProtocolXML?"         +guidURL;
            addrSave= splUrl[0] +"/SetProtocolResultsXML";
        }

        Web.connection_Server = true;
        subscribe();
    }


   Component.onCompleted: {
//       Web.connection_Server = true;
//       lsTimer.running       = true;

//       var xsltFile= "import_lis_file.xsl"
//       var xmlFile = "1164.xml"

//       console.log( xtItem.transfomXSLT(xmlFile, xsltFile) )

//       lsTimer.start();
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
    */
   /*
    }
*/

/*
   Rectangle
   {
        id: control_status
        x:  5
        y:  5
        color:  "red"
        width:   10
        height:  10
   }   
*/

   function subscribe(){

        lsTimer.start();
        return "";
   }


   function getListProtocols(){

       print( "GET--> "+ addrList );

       var doc = new XMLHttpRequest();

       print( "XMLHttpRequest init");

       doc.onreadystatechange = function() {

            print( doc.readyState + " state of socket" );

            if (doc.readyState === XMLHttpRequest.DONE) {

               if (doc.responseXML === null){
                   console.log("Unable to get responce");
                   return;
               }

               var a = doc.responseXML.documentElement;
               var sjoined = [];

               for (var ii = 0; ii < a.childNodes.length; ++ii) {

                   if( a.childNodes[ii].nodeName === "Protocol"
                   &&  a.childNodes[ii].attributes !== null ){

                       sjoined.push( [ a.childNodes[ii].attributes['ID'].value
                                      ,a.childNodes[ii].attributes['GUID'].value
                                      ,a.childNodes[ii].attributes['Name'].value
                                      ].join(';')
                                    );
                   }
               }

               print( sjoined.length + " elements in list" );

               Web.get_ListProtocols( sjoined.join(':'), ':' );
            }
        }

        doc.open("GET", addrList );
        doc.send();
   }


   function getWebProtocol(id_protocol)
   {
       if ( !id_protocol.length ){
           return getListProtocols();
       }

       //print("getWebProtocol");


       var doc      = new XMLHttpRequest();
       var addrScr  = root.addrProt + "&ProtocolID="+id_protocol

       doc.onreadystatechange = function() {

           if (doc.readyState === XMLHttpRequest.DONE) {

               if (doc.responseXML === null){
                   console.log("Unable to get responce");
                   return;
               }

               var str = doc.responseText;
               Web.get_Protocol( str );

//               transfomXSLT( str )
           }
       }

       doc.open("GET", addrScr);
       doc.send();
   }


   function send_ProtocolToWeb(id_protocol, data_protocol, name_protocol)
   {
       //return; // TODO: !!!!!!!!

       var doc      = new XMLHttpRequest();
       var addrPost = addrSave;

       doc.onreadystatechange = function() {

           var ret = {
                    request: "-1",  protocol: id_protocol,
                    status: "1",    message: "",
                    build: function(){
                        var r = String(
                            '<?xml version="1.0" encoding="utf-8" standalone="yes"?>'
                           +'<request id="%%R%%">'
                           +  '<name>SaveResultsCallback</name>'
                           +  '<id_protocol>%%P%%</id_protocol>'
                           +  '<status>%%S%%</status>'
                           +  '<msg>%%M%%</msg>'
                           +'</request>');

                        return r.replace( "%%R%%", this.request )
                                .replace( "%%P%%", this.protocol )
                                .replace( "%%S%%", this.status )
                                .replace( "%%M%%", Qt.btoa(this.message) );
                    }
           };

           if (doc.readyState === XMLHttpRequest.DONE) {

               if (doc.responseXML === null){
                   ret.message= String('<package><Error Message="POST-ing %%P%% is unavailable"</Error></package>')
                                .replace("%%P%%", name_protocol);
               }else{
                   ret.status = "0";
                   ret.message= doc.responseText;
               }

               Web.exec_Command( ret.build() );
               //console.log(doc.responseText);
           }
       }

       doc.open("POST", addrPost);
       doc.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
       doc.send( accountGUID + "&XML=" + encodeURI( data_protocol ));
   }

   function send_StateProtocolToWeb( id_protocol, state_protocol )
   {
        console.log( "Fake Sending state '"+state_protocol+"' of protocol "+ id_protocol );
   }


   function set_StateSocket(state)
   {
//       socket.active = state;
       lsTimer.running = state;
   }

   // TODO: comment Item declaration and replace 'web' to 'Web' inside this script
   //
//   Item{
//       id: web
//       property bool connection_Server: false
//       function get_ListProtocols(str,sep){ console.log("get_ListProtocols") }
//       function get_Protocol(str){ console.log("get_Protocol") }
//   }


   Item{
       id: iClient
       property string  md5List: "-1"

       Timer{
           id:  lsTimer
           interval:    10000;
           running:     false;
           repeat:      true
           triggeredOnStart:true;

           onTriggered: { getListProtocols(); }
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
//            socket.active = !socket.active;
            lsTimer.running = !lsTimer.running;
            button.text = lsTimer.running? "Disconnect": "Connect.."
        }

        Component.onCompleted:
        {
            button.text = lsTimer.running? "Disconnect": "Connect.."
        }
   }
*/


}


