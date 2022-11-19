#include <https_web_server.h>

#include <ESPmDNS.h>

#include <sstream>
#include <cstdint>

uint64_t latestLatency = 0;
void HttpsWebServer::setupDNS(){
      if (!MDNS.begin("projektionfx")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("mDNS responder started");
}

void HttpsWebServer::generateSSLCert(){
  Serial.println("Creating certificate... (takes ~30s)");
  
  cert = new SSLCert();

  int createCertResult = createSelfSignedCert(
    *cert,
    SSLKeySize::KEYSIZE_2048,
    "CN=projektionfx.local,O=acme,C=DE");
  
  if (createCertResult != 0) {
    Serial.printf("Error generating certificate");
    return; 
  }

  Serial.println("Certificate created with success");
}

void HttpsWebServer::registerDelayRoute(){
  ResourceNode * nodeStreamDelay = new ResourceNode("/delay", "GET", [](HTTPRequest * req, HTTPResponse * res){
    ResourceParameters * params = req->getParams();
    std::string ParamLatencyName = "latency";
    std::string requestLatency;
    if (!params->getQueryParameter(ParamLatencyName, requestLatency)){
      Serial.println("latency param not found");
      return;
    }

    Serial.print("Current stream latency: ");
    Serial.print(requestLatency.c_str());
    Serial.println("ms");
    std::istringstream iss(requestLatency);
    iss >> latestLatency;
  });
  secureServer->registerNode(nodeStreamDelay);
}

void HttpsWebServer::registerDefaultRoute(){
  ResourceNode * nodeRoot = new ResourceNode("/", "GET", [](HTTPRequest * req, HTTPResponse * res){
    res->setHeader("Content-Type", "text/html");
    String s;
    s += ("<!DOCTYPE html><html>\r\n");
    s += ("<head>\r\n");
    s += ("<meta name='viewport' content='width=device-width, initial-scale=1'>\r\n");
    s += ("<script src='https://player.twitch.tv/js/embed/v1.js'></script>\r\n");
    s += ("</head>\r\n");
    s += ("<body>\r\n");
    s += ("<div id='twitchplayer'></div>\r\n");
    s += ("<br>\r\n");
    s += ("<div id='delay'></div>\r\n");
    s += ("<script type='text/javascript'>\r\n");
    s += ("    //countdown function is evoked when page is loaded\r\n");
    s += ("    var lastLatency = 0;\n");
    s += ("    function latencyCheck() {\r\n");
    s += ("        var latency = parseInt(player.getPlaybackStats().hlsLatencyBroadcaster * 1000)\r\n");
    s += ("        document.getElementById('delay').innerHTML = latency;\r\n");
    s += ("        if (lastLatency != latency)\r\n");
    s += ("        {\r\n");
    s += ("            fetch('/delay?latency='+latency);\r\n");
    s += ("            lastLatency = latency;\r\n");
    s += ("        }\r\n");
    s += ("    }\r\n");
    s += ("    var options = {\r\n");
    s += ("        width: 1280,\r\n");
    s += ("        height: 720,\r\n");
    s += ("        channel: 'lastmiles',\r\n");
    s += ("    };\r\n");
    s += ("    var player = new Twitch.Player('twitchplayer', options);\r\n");
    s += ("    player.setVolume(0.5);\r\n");
    s += ("    setInterval(latencyCheck, 1000);\r\n");
    s += ("</script>\r\n");
    s += ("</body>\r\n");
    s += ("</html>\r\n");
    s += "\r\n\r\n";
    res->println(s);
  });
  secureServer->registerNode(nodeRoot);
}

boolean  HttpsWebServer::start(){
  secureServer = new HTTPSServer(cert);
  registerDefaultRoute();
  registerDelayRoute();
  secureServer->start();

  if (secureServer->isRunning()) {
    return false;
  }
  Serial.println("Server ready.");
  return true;
}

void HttpsWebServer::loop(){
  secureServer->loop();
  this->streamLatency = latestLatency;
}