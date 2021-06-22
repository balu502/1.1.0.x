import QtQuick 2.0
import QtQuick.Controls 2.0
import QtWebSockets 1.0
//import com.dna_technology.Web_Interface 1.0

Rectangle
{
   color:   "white"
   width:   200
   height:  200

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
       var str = "subscribe:consumer data requested:consumer data available:consumer status requested:consumer cancel requested"
       var encodedString = Qt.btoa(str);
       var Str = "event:subscribe:client:rtapp:data:" + encodedString;

       //console.log(str);
       //console.log(encodedString);

       return Str;
   }

   WebSocket
   {
       id: socket
       //url: "ws://localhost:9002"
       url: "ws://192.168.0.108:9002"
       active: false


       onStatusChanged:
       {
           var str = "";           

           if(socket.status == WebSocket.Error)  console.log("Error: " + socket.errorString)
           else
           {
               if(socket.status == WebSocket.Open)
               {
                   control_status.color = "green";
                   button.text = "Disconnect...";                   
                   Web.connection_Server = true;
                   str = subscribe();
                   //print(str);
                   socket.sendTextMessage(str);
               }
               if(socket.status == WebSocket.Closed)
               {
                   control_status.color = "red";
                   button.text = "Connect...";
                   Web.connection_Server = false;
               }
           }
       }
       onTextMessageReceived:
       {
           var array_str = message.split(":");
           var event_type = array_str[1];
           var str;

           switch(event_type)
           {
            default:                            break;

            case "subscribe":                   break;
            case "consumer data available":
                {
                    str = Qt.atob(array_str[9]);
                    Web.get_ListProtocols(str,":");
                    break;
                }
           }
       }
   }

   Button
   {
        id: button
        anchors.centerIn: parent
        text:   "Connect..."
        width:  parent.width/2;
        height: parent.height/2;

        onClicked:
        {
            socket.active = !socket.active;            
        }
   }


}


