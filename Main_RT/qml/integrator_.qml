import QtQuick 2.4          // TODO: import QtQuick 2.0
import QtQuick.Controls 1.0
//import WSDir 1.0

Rectangle
{
    id:     root
    color:  "white"
    width:  200
    height: 200

    property string accountGUID: "7c890b798cd940999a4bbd529aa1e6e0"

//    property string addrList: "http://192.168.0.167:8080/kisprotocol.php"
    property string addrList: "http://10.1.1.48/GetProtocolListXML?AccountGUID="+accountGUID
    property string addrProt: "http://10.1.1.48/GetProtocolXML?AccountGUID="+accountGUID
    property string addrSave: "http://10.1.1.48/SetProtocolResultsXML"


   Component.onCompleted: {
       Web.connection_Server = true;
//       lsTimer.running       = true;

       var xsltFile= "import_lis_file.xsl"
       var xmlFile = "1164.xml"

       console.log( xtItem.transfomXSLT(xmlFile, xsltFile) )
   }


   /*Item {

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

   }*/

    /*Item {

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

    }*/


   Rectangle
   {
        id: control_status
        x:  5
        y:  5
        color:  "red"
        width:   10
        height:  10
   }   

   function subscribe()
   {
       return "";
   }


//   function getListProtocols(){

//       var doc = new XMLHttpRequest();

//       doc.onreadystatechange = function() {

//           if (doc.readyState === XMLHttpRequest.DONE) {

//               if (doc.responseXML === null){
//                   console.log("Unable to get responce");
////                            lsTimer.running = false;
//                   return;
//               }

//               var a = doc.responseXML.documentElement;
//               var e = doc.getResponseHeader("Etag");

//               if( iClient.md5List !== e ){
//                   iClient.md5List = e;

//                   var sjoined = [];

//                   for (var ii = 0; ii < a.childNodes.length; ++ii) {

//                       if( a.childNodes[ii].nodeName === "li"
//                       &&  a.childNodes[ii].attributes !== null ){

//                           sjoined.push( [ a.childNodes[ii].attributes['idtask'].value
//                                          ,a.childNodes[ii].attributes['barcode'].value
//                                          ,a.childNodes[ii].attributes['name'].value
//                                          ].join(';')
//                                        );
//                       }
//                   }

//                   web.get_ListProtocols( sjoined.join(':'), ':' );
//               }
//           }
//       }

//      doc.open("GET", root.addrList);
//      doc.send();
//   }


   function getListProtocols(){

       var doc = new XMLHttpRequest();

       doc.onreadystatechange = function() {

           if (doc.readyState === XMLHttpRequest.DONE) {

               if (doc.responseXML === null){
                   console.log("Unable to get responce");
//                            lsTimer.running = false;
                   return;
               }

               var a = doc.responseXML.documentElement;
//               var e = doc.getResponseHeader("Etag");

//               if( iClient.md5List !== e ){
//                   iClient.md5List = e;

                   var sjoined = [];

                   for (var ii = 0; ii < a.childNodes.length; ++ii) {

//                       console.log(ii);

                       if( a.childNodes[ii].nodeName === "Protocol"
                       &&  a.childNodes[ii].attributes !== null ){

                           sjoined.push( [ a.childNodes[ii].attributes['ID'].value
                                          ,a.childNodes[ii].attributes['GUID'].value
                                          ,a.childNodes[ii].attributes['Name'].value
                                          ].join(';')
                                        );
                       }
                   }

                   Web.get_ListProtocols( sjoined.join(':'), ':' );
//               }
           }
       }

      doc.open("GET", root.addrList);
      doc.send();
   }


   function getWebProtocol(id_protocol)
   {
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
       doc.send()
   }


   function send_ProtocolToWeb(id_protocol, data_protocol)
   {
       return; // TODO: !!!!!!!!

       var doc      = new XMLHttpRequest();
       var addrPost = root.addrSave;

       doc.onreadystatechange = function() {

           if (doc.readyState === XMLHttpRequest.DONE) {

               if (doc.responseXML === null){
                   console.log("Unable to get POST responce");
                   return;
               }
//               console.log(doc.responseText);
           }
       }

       doc.open("POST", addrScr);
       doc.setRequestHeader("Content-Type", "application/x-www-form-urlencoded")
       doc.send("AccountGUID=" + accountGUID + "&XML=" + encodeURI( data_protocol ))
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
           id:lsTimer
           interval: 5000; running: false; repeat: true
           triggeredOnStart:true;

           onTriggered: { getListProtocols(); }
       }
   }


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


}


