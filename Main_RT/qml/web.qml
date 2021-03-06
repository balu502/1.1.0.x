import QtQuick 2.0
import QtWebSockets 1.0


Item
{
   function subscribe()
   {
       if(!Web.connection_Server) return;

       var ctype= socket.clientType;
       var str = "subscribe"
             +   ":consumer data requested"
             +   ":consumer data available"
             +   ":consumer status requested"
             +   ":consumer cancel requested"
       var encodedString = Qt.btoa(str);
       var Str = "event:subscribe"
             +   ":client:"  + ctype
             +   ":data:"    + encodedString;


       Web.send_Message(str);
       return Str;
   }

   function getWebProtocol(id_protocol)
   {
       if(!Web.connection_Server) return;

       var str = "event:amplif data requested:subject:" + id_protocol;
       socket.sendTextMessage(str);
       Web.send_Message(str);
   }

   function send_ProtocolToWeb(id_protocol, data_protocol, name_protocol)
   {
       if(!Web.connection_Server) return;

       var str = Qt.btoa(data_protocol);
       var Str = "event:amplif data available:subject:" + id_protocol + ":data:";
       socket.sendTextMessage(Str + str);
       Web.send_Message(Str + " (" + name_protocol + ")");
   }

   function send_ProtocolOrderToWeb(id_protocol, data_protocol, name_protocol)      // 1 june 2019
   {
       if(!Web.connection_Server) return;

       var str = Qt.btoa(data_protocol);
       var Str = "event:consumer data available:subject:" + id_protocol + ":data:";
       socket.sendTextMessage(Str + str);
       //Web.send_Message(Str + " (" + name_protocol + ")");
   }

   function send_StateProtocolToWeb(id_protocol, state_protocol)
   {
       if(!Web.connection_Server) return;

       var message = " (" + id_protocol + ";" + state_protocol + ")";

       var str = ":data:" + Qt.btoa(state_protocol);
       var Str = "event:amplif status changed:subject:" + id_protocol + str;
       socket.sendTextMessage(Str);
       Web.send_Message(Str + message);
   }

   function set_StateSocket(state)
   {
       //socket.url = "ws://192.168.0.60:9002";


       socket.active = !socket.active; //state;
       //print("socket state: " + socket.active);
       //socket.active = state;
   }

   function set_SocketUrl(url)
   {
        //print("set_SocketUrl(url): " + url);
        //socket.active = true;
        socket.url = url;

   }

   Timer {
          id: reconnectTimer

          interval:50000;
          running: false;
          repeat:  true

          onTriggered: {

              if(( socket.status == WebSocket.Error )
               ||( socket.status == WebSocket.Closed )){

                  console.log("Connection status is:"
                              + socket.status
                              +". Try reconnect" );

                  socket.active = false;
                  socket.active = true;

              }else stop();
          }
   }


   WebSocket
   {
       id: socket
       url: "" //"ws://localhost:9902"
       //url: "ws://192.168.0.51:9902"
       active: true
	   property string connectionId: ""
       property string clientType:  "rtapp"


       Component.onCompleted:
       {
           //print("create Websocket:" );

       }

       onStatusChanged:
       {
           var str = "";
           //print("WebSocket status:" + socket.status);

           if(socket.status == WebSocket.Error)
           {
               console.log("Error: " + socket.errorString);
               active && reconnectTimer.start();
           }
           else
           {

               if(socket.status == WebSocket.Open)
               {                   
                   //print("socket.url: " + socket.url);

                   Web.url_Server = socket.url;
                   Web.connection_Server = true;

                   connectionId = "";
                   str = subscribe();                   
                   socket.sendTextMessage(str);
               }
               if(socket.status == WebSocket.Closed)
               {                   
                   Web.connection_Server = false;
                   connectionId = "";
               }
           }
       }
       onTextMessageReceived:
       {
           var array_str = message.split(":");
           var event_type = array_str[1];
           var subject_type = array_str[array_str.indexOf( "subject")+1];
		   var rcr 			= array_str[array_str.indexOf("receiver")+1];
		   var sdr 			= array_str[array_str.indexOf(  "sender")+1];
           var str;
           var str_param = "";



           switch(event_type)
           {
            default:                            break;

            case "subscribe":
				{
                    if ( (sdr==rcr) && !connectionId.length ){
						connectionId = rcr;
						print("Subscribed with id: " + connectionId);
					}
					break;
				}
            case "consumer data available":
                {
                    print("sender:"+sdr+", receiver:"+rcr+", connectionId:"+connectionId)

                    str = Qt.atob(array_str[9]);
					
                    if(subject_type == ""){		// get list of protocols from server
							Web.get_ListProtocols(str,":"); 
							str_param = " :List of Protocols";
					}else {						
												// ???????? ????? ???????? ?? ???????, 
												// ?????????, ???? ?????????? ?? ??? connectionId, 
												// ?? ????????????
							if ( rcr == connectionId ){

                                print( "Got protocol" );

								Web.get_Protocol(str); str_param = " :Protocol";
                                //str = event_type + str_param;
                                //Web.get_Message(str);
							}
					}

                    //print(str);
                    break;
                }
           }
           // str = event_type + str_param;
           // Web.get_Message(str);
           //print(message)
       }
   }

}


//}


