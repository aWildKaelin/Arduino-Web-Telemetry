#include <iostream>
#include <string.h>
#include <string>
#include <unordered_map>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/buffer.hpp>

#include <nlohmann/json.hpp>


asio::io_context context; //like a GL context

// every socket needs an acceptor to actually receive new objects
// one for website, one for robot
asio::ip::tcp::socket HTMLsocket(context);
asio::ip::tcp::acceptor HTMLacceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 80));

asio::ip::tcp::socket mouseSocket(context);
asio::ip::tcp::acceptor mouseAcceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8084));


std::unordered_map<std::string, int> receiveStorage;  // receive from robot
std::unordered_map<std::string, int> sendStorage;     // send to robot

std::string website = R"html(
HTTP/1.1 200 OK
Content-Type: text/html
Connection: close

<!DOCTYPE html>
<html>
    <head>
        <title>Robot Interface</title>
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <style>
            body { font-family: system-ui, sans-serif; margin: 1rem; }
            .row { display: flex; gap: 1rem; align-items: center; margin: .5rem 0; }
            input[type=number] { width: 6rem; }
        </style>
    </head>

    <body>
        <h1>Robot Name</h1>
        <hr>

        <section id="varDisplay">
        </section>
        
        <br>

        <section id="varAdjust">
        </section>

    </body>

    <script>
        function updateWebsite(){
            fetch('/update')
            .then(r => r.json())
            .then(data => {
                let varDisplay = document.getElementById("varDisplay");
                
                Object.keys(data).forEach(key => {
                    console.log(`${key}: ${data[key]}`);
                    let element = document.getElementById(`${key}`);;
                    if(element)
                        element.textContent = `${key}: ${data[key] / 100}`;
                    else
                    {
                        let newElement = document.createElement("p");
                        newElement.id = `${key}`;
                        newElement.textContent = `${key}: ${data[key] / 100}`;
                        varDisplay.appendChild(newElement);
                    }
                });
            });
        }

        fetch('/setup')
            .then(r => r.json())
            .then(data => {
                let varDisplay = document.getElementById("varAdjust");
                
                Object.keys(data).forEach(key => {
                    console.log(`${key}: ${data[key]}`);

                    let newLabel = document.createElement("label");
                    newLabel.textContent = `${key}: `;
                    varDisplay.appendChild(newLabel);

                    let newInput = document.createElement("input");
                    newInput.name = `${key}`;
                    newInput.type = "number";
                    newInput.step = ".01";
                    newInput.value = `${data[key] / 100}`;
                    varDisplay.appendChild(newInput);

                    varDisplay.appendChild(document.createElement("br"));
                });
            });



        console.log("Hello from JavaScript!");
        
        var intervalID = setInterval(updateWebsite, 500);
        
        
        document.querySelectorAll("input").forEach(el => {
            el.addEventListener("input", numInputHandler);
        });

        function numInputHandler(e){
            const param = e.target.name;
            const value = Number(e.target.value);
            console.log(param, value);
            fetch("/update", {
                method: "PUT",
                body: JSON.stringify({varName: param, var: (value * 100)}),
            });
        }
    </script>
</html>
)html";

std::string robotBuffer;
char tempBuffer[1024];  // used to ensure that all data arrives before processing to prevent errors

void enqueueWebInterface()
{
    HTMLacceptor.async_accept(HTMLsocket,
        [&](asio::error_code ec) {
            if (!ec)
            {
                char receiveBuffer[1024];
                asio::error_code read_ec;
                size_t bytes = HTMLsocket.read_some(asio::buffer(receiveBuffer), read_ec);

                receiveBuffer[bytes] = '\0';
                std::string buffer(receiveBuffer);
                std::string sendData;

                if (buffer.find("GET") != std::string::npos)
                {
                    if (strstr(buffer.c_str() + 3, "/ ") != 0)
                    {
                        sendData = website;

                    }
                    else if (strstr(buffer.c_str() + 3, "/update") != 0)
                    {
                        sendData = "HTTP/1.1 200 OK\r\nContent-Type: text/json\r\n\r\n";
                        nlohmann::json data;
                        for (auto el : receiveStorage) data[el.first] = el.second;
                        sendData += data.dump();
                    }
                    else if (strstr(buffer.c_str() + 3, "/setup") != 0)
                    {
                        sendData = "HTTP/1.1 200 OK\r\nContent-Type: text/json\r\n\r\n";
                        nlohmann::json data;
                        for (auto el : sendStorage) data[el.first] = el.second;
                        sendData += data.dump();
                    }
                    else
                    {
                        std::cout << "Invalid GET endpoint! Dumping:\n" << buffer << std::endl;
                        sendData = "HTTP/1.1 404 Not Found\r\n";
                    }

                    HTMLsocket.write_some(asio::buffer(sendData.data(), sendData.size()), ec);
                }
                else if ((buffer.find("PUT") != std::string::npos))
                {
                    if (strstr(buffer.c_str() + 3, "/update") != 0)
                    {
                        std::cout << "PUT:\n" << buffer << std::endl;
                        sendData = "HTTP/1.1 200 OK\r\n";

                        size_t jsonIndex = buffer.find('{') != std::string::npos ? buffer.find('{') : NULL;
                        if (jsonIndex == NULL) std::cout << "NO JSON DATA AVAILABLE \n";
                        else {
                            nlohmann::json data = nlohmann::json::parse(buffer.c_str() + jsonIndex - 1);
                            
                            for (auto& el : data.items())
                            {
                                std::cout << "key: " << el.key() << ", value:" << el.value() << '\n';
                            }
                        }
                    }
                    else
                    {
                        std::cout << "Invalid PUT endpoint! Dumping:\n" << buffer << std::endl;
                        sendData = "HTTP/1.1 404 Not Found\r\n";
                    }
                    HTMLsocket.write_some(asio::buffer(sendData.data(), sendData.size()), ec);
                }
                else 
                {
                    std::cout << "Unknown REST symbol! Dumping:\n" << buffer << std::endl;
                }

                
                HTMLsocket.close();

                enqueueWebInterface();
            }
            else std::cout << "HTML Socket async_accept error! - \n" << ec.message() << std::endl;
        });
}


void enqueueRobotInterface()
{
    mouseSocket.async_read_some(asio::buffer(tempBuffer, 1024), [&](asio::error_code ec, std::size_t bytesTransferred) {
        if (!ec)
        {
            robotBuffer.append(tempBuffer, bytesTransferred);
            if (robotBuffer.find("\r\n") != std::string::npos)
            {
                std::cout << "Got robot data:\n" << robotBuffer << std::endl;

                while (robotBuffer.size() != 0)
                {
                    if (robotBuffer[0] == '\r' && robotBuffer[1] == '\n')
                    {
                        robotBuffer.clear();
                        // do send to robot code
                    }
                    else
                    {
                        int* ref = &receiveStorage[robotBuffer.substr(0, robotBuffer.find(',') != std::string::npos ? robotBuffer.find(',') : robotBuffer.size())];
                        robotBuffer.erase(0, robotBuffer.find(',') != std::string::npos ? robotBuffer.find(',') + 1 : robotBuffer.size());
                        *ref = atoi(robotBuffer.c_str());
                        robotBuffer.erase(0, robotBuffer.find(',') != std::string::npos ? robotBuffer.find(',') + 1 : robotBuffer.size());
                    }
                }

                //std::string sendData = "receive acknowledge\n";

                //mouseSocket.write_some(asio::buffer({ 0 }, 1), ec);
                //if(ec) std::cout << "Socket write_some error! - \n" << ec.message() << std::endl;
            }
            enqueueRobotInterface();
        }
        else std::cerr << "Robot Socket async_read_some error! - \n" << ec.message() << std::endl;
    });
}


int main(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        std::cout << argv[i] << std::endl;
    }


    mouseAcceptor.accept(mouseSocket);

    while (!mouseSocket.available());
    std::cout << "init msg: ";
    std::string setup;
    while (mouseSocket.available()) {
        mouseSocket.read_some(asio::buffer(tempBuffer, 1024));
        setup += tempBuffer;
    }
    std::cout << setup;

    while (setup.size() != 0)
    {
        sendStorage[setup.substr(0, setup.find(',') != std::string::npos ? setup.find(',') : setup.size())] = 0;
        setup.erase(0, setup.find(',') != std::string::npos ? setup.find(',') + 1 : setup.size());
    }

    enqueueWebInterface();
    enqueueRobotInterface();
    
    context.run(); // runs the queued tasks

    return 0;
}