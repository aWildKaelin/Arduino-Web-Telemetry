#include <iostream>
#include <string.h>
#include <string>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/buffer.hpp>


asio::io_context context; //like a GL context
asio::ip::tcp::socket HTMLsocket(context);
asio::ip::tcp::acceptor HTMLacceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 80));

asio::ip::tcp::socket mouseSocket(context);
asio::ip::tcp::acceptor mouseAcceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8084));

std::string robotBuffer;
char tempBuffer[1024];

bool swapped = true;

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
                        sendData = R"html(
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

        <label for="in">input 1:</label>
        <input type="number" name="in1" step=".01" value="0">
        <br>
        <label for="in">input 2:</label>
        <input type="number" name="in2" step=".01" value="0">

        <button type="button">Click Me!</button> 

        <p id="textDisplayOne"></p>

        
    </body>

    <script>
        function updateWebsite(textDisplayOne){
            fetch('/update')
            .then(r => r.json())
            .then(data => textDisplayOne.textContent = data.test);
        }

        console.log("Hello from JavaScript!");
        const textDisplayOne = document.getElementById("textDisplayOne");
        textDisplayOne.textContent = "bingus";
        

        document.querySelectorAll("input").forEach(el => {
            el.addEventListener("input", numInputHandler);
        });

        function numInputHandler(e){
            const param = e.target.name;
            const value = Number(e.target.value);
            console.log(param, value);
            fetch("/update", {
                method: "PUT",
                body: JSON.stringify({varName: param, var: value}),
            });
        }
        
    </script>
</html>
)html";

                    }
                    else if (strstr(buffer.c_str() + 3, "/update") != 0)
                    {
                        if (swapped)
                        {
                            sendData =
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/json\r\n\r\n"
                                "{\"test\":\"this is a test string\"}\r\n";
                        }
                        else
                        {
                            sendData =
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/json\r\n\r\n"
                                "{\"test\":\"this is another test string\"}\r\n";
                        }
                        swapped = !swapped;
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
            else std::cout << ec.message() << std::endl;
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

                std::string sendData = "receive acknowledge\n";

                mouseSocket.write_some(asio::buffer({ 0 }, 1), ec);
                if(ec) std::cout << "write_some check - " << ec.message() << std::endl;
            }
            enqueueRobotInterface();
        }
        else std::cerr << "general check - " << ec.message() << std::endl;
    });
}


int main(int argc, char** argv)
{
    /*
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("104.230.16.86", ec), 17);
    asio::ip::tcp::socket socket(context);

    socket.connect(endpoint, ec); //reach endpoint
    if (ec) std::cout << "failed to connect:\n" << ec.message() << std::endl;
    else std::cout << "Connected!\n" << std::endl;

    if (socket.is_open()) //if the socket is opened, not necessarily connected
    {
        std::string request = "hello from basic ASIO test\r\n";

        //send
        socket.write_some(asio::buffer(request.data(), request.size()), ec);

        if (ec) std::cout << "Error!:\n" << ec.message() << std::endl;

        //wait for data to appear
        while (!socket.available());
        size_t bytes = socket.available();

        //receive
        std::vector<char> vBuffer(bytes);
        socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

        for (auto c : vBuffer)
            std::cout << c;
    }
    */


    /*
    asio::ip::tcp::acceptor acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 80));
    asio::ip::tcp::socket socket(context);

    acceptor.accept(socket);

    while (!socket.available());
    size_t bytes = socket.available();

    std::vector<char> vBuffer(bytes);
    socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

    for (auto c : vBuffer)
        std::cout << c;

    std::string sendData = "HTTP/1.1 200 OK\r\n";

    socket.write_some(asio::buffer(sendData.data(), sendData.size()), ec);
    socket.close();
    */



    //mouseAcceptor.accept(mouseSocket);

    enqueueWebInterface();
    //enqueueRobotInterface();
    
    context.run(); // runs the queued tasks

    //while (true);

    return 0;
}