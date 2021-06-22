import QtQuick 2.3
import QtWebSockets 1.0
import QtQuick.XmlListModel 2.0


Rectangle
{
    id: main;
    width: 1;
    height: 1;
    color: "black"


    function subscribe()
    {
       if(!Web.connection_Server) return;

       var str = "subscribe"
                    +":dropper data requested"
                    +":dropper data available"
                    +":dropper status requested"
                    +":dropper cancel requested"
                    +":amplif data available"
                    +":consumer command"
                    ;

       var encodedString = Qt.btoa(str);
       var ctype= socket.clientType;
	
       var evt = "event:subscribe"
                +    ":client:"  + ctype
                +    ":data:"    + encodedString
                ;

       Web.send_Message(str);
       return evt;
    }

    function getWebProtocol(id_protocol)
    {
       //if(!Web.connection_Server) return;

       var str = "event:dropper data requested:subject:" + id_protocol;
       socket.sendTextMessage(str);
       Web.send_Message(str);
    }

    /*
    *	Запрос списка исполненных протоколов с сервера
    * 	! 5 июля id_protocol=""
    */
    function getWebCompleted(id_protocol)
    {
       if(!Web.connection_Server) return;

       var str = "event:amplif data requested:subject:" + id_protocol;
       socket.sendTextMessage(str);
       Web.send_Message(str);
    }

    function send_ProtocolToWeb(id_protocol, data_protocol, name_protocol)
    {
       if(!Web.connection_Server) return;

       var str = Qt.btoa(encodeURIComponent(data_protocol));
       var rcr = socket.getMember( "datastorage", "0" );
       var evt = "event:amplif data available"
                    + ":receiver:"+ rcr
                    + ":subject:" + id_protocol
                    + ":data:";

       socket.sendTextMessage(evt + str);
       Web.send_Message(evt + " (" + name_protocol + ")");
    }

    function send_StateProtocolToWeb(id_protocol, state_protocol)
    {
       if(!Web.connection_Server) return;

       var message = " (" + id_protocol + ";" + state_protocol + ")";

       var str = ":data:" + Qt.btoa(state_protocol);
       var evt = "event:amplif status changed:subject:" + id_protocol + str;
       socket.sendTextMessage(evt);
       Web.send_Message(evt + message);
    }

    function set_StateSocket(state)
    {
       //socket.url = "ws://192.168.0.60:9002";
       socket.active = !socket.active; //state;
    }

    function set_SocketUrl(url)
    {
        socket.url = url;
    }


    Timer {
       id: reconnectTimer

       interval:5000;
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
        //url: "ws://172.16.0.60:9902"	// for testing
        url: ""
        active: true
	    property string connectionId: ""
        property string clientType:  "rtappD"
        property var membersArray: []

        function getMember(memName, defaultId){
            return (membersArray[memName] === undefined)
                    ? defaultId: membersArray[memName];
        }

        function updateMembersList( mbsData ){
            var mms = String(decodeURIComponent(Qt.atob(mbsData)));
                mms = mms.split(":");
            for(var m in mms) if(m%2) membersArray[ mms[m-1] ] = mms[ m ];
        }


        onStatusChanged:
               {
                   var str = "";

                   if(socket.status == WebSocket.Error)
                   {
					   print("Error_socket: " + socket.errorString);
					   print("While connect to : " + socket.url);
                       active && reconnectTimer.start();
						//console.log("Error: " + socket.errorString)
                   }
                   else
                   {
                       if(socket.status == WebSocket.Open)
                       {
                           print("web address: " + socket.url);

                           Web.url_Server = socket.url;
                           Web.connection_Server = true;

                           connectionId = "";
                           str = subscribe();
                           socket.sendTextMessage(str);
                       }
                       if(socket.status == WebSocket.Closed)
                       {
                           Web.connection_Server = false;

                       }
                   }
               }
	   // onTextMessageReceived:
	   // {
		   // var array_str = message.split(":");
		   // var event_type = array_str[1];
		   // var subject_type = array_str[array_str.indexOf("subject")+1]
		   // var str;
		   // var str_param = "";

		   // switch(event_type)
		   // {
			// default:                            break;

			// case "subscribe":                   break;
			// case "consumer data available":
				// {
					// str = Qt.atob(array_str[9]);
                    // if(subject_type == ""){Web.get_ListProtocols(str,":"); str_param = " :List of Protocols";}
                    // else {Web.get_Protocol(str); str_param = " :Protocol";}

					// break;
				// }
		   // }
		   // str = event_type + str_param;
           // Web.get_Message(str);
		   // //print(message)
	   // }
			   
		onTextMessageReceived:
		{
			var array_str = message.split(":");
			var event_type 	= array_str[1];
			var subject_type= array_str[array_str.indexOf( "subject")+1];
			var rcr 		= array_str[array_str.indexOf("receiver")+1];
			var sdr 		= array_str[array_str.indexOf(  "sender")+1];
			var str;
			var str_param = "";

			print( event_type+", subject:"+subject_type
					+", sender:"+sdr+", receiver:"+rcr
					+", connectionId:"+connectionId );

		   switch(event_type)
		   {
			default:                            
				break;

			case "subscribe":
				{
					if ( (sdr==rcr) && !connectionId.length ){
						connectionId = rcr;
						print("Subscribed with id: " + connectionId);
					}

                    // member enter/exit chat, update members list
                    //
                    updateMembersList(array_str[9]);

                    if( "-1" === socket.getMember( "datastorage", "-1" )){
                        // Если хранилище отключено, то обнулить
                        // список протоколов и запросить его у капалок.
                        Web.get_ListProtocols("", ":");
                        getWebProtocol("");
                    }

					break;
				}
				
			case "dropper data available":
				{
                    var sdd = socket.getMember( "datastorage", "-1" );

                    if ((sdd !== "-1")  // Принимаем от капалки, только
                    &&  (sdd !== sdr )) // если в сети нет хранилища
                        break;

					str = decodeURIComponent(Qt.atob(array_str[9]));
					
					if(subject_type == ""){		// получили список протоколов от сервера
                            Web.get_ListProtocols(str,":");
							str_param = " :List of Protocols";
					}else {						
												// получили некий протокол от сервера, 
												// проверяем, если отправлено на наш connectionId, 
												// то обрабатываем
							if ( rcr == connectionId ){
                                Web.get_Protocol(str);
								str_param = " :Protocol";
							}
					}

					//print(str);
					break;
				}
				
			case "amplif data available":
				{
					str = decodeURIComponent(Qt.atob(array_str[9]));
					
					if(subject_type == ""){		// получили список исполненных протоколов от сервера
												// ! 5 июля
                            Web.get_ListCompleted(str,":");
							str_param = " :List of Protocols";
					}else {						
												// получили некий протокол от сервера, 
												// проверяем, если отправлено на наш connectionId, 
												// то обрабатываем
							/*
							if ( rcr == connectionId ){
                                Web.get_Completed(str);
								str_param = " :Protocol";
							}
							*/
					}

					//print(str);
					break;
				}					
		   
		   
			case "consumer command":
				{
                    if ( connectionId !== rcr ){
                        print( "Command pass" );
                        break;
                    }

					// Testing only
					//
					var cmdcode = subject_type;

                    if ("1" == cmdcode){

                        var data_protocol = localFile.contentainerStr;
                                //decodeURIComponent( Qt.atob(array_str[9]) );
                        send_ProtocolToWeb( "19", data_protocol, "quality.r96" );
					}
					
					break;
				}
		   }
		   
		   // str = event_type + str_param;
           // Web.get_Message(str);
		   //print(message)
		}
    }

    Item{
        Loader {
            id: localFile
            property string contentainerStr: ""
            source: "WData.qml";
            onStatusChanged: console.log("status", status,  "item", item)
            onLoaded: contentainerStr = String(item.contentainerStr);
        }
    }
}


