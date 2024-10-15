The approach revolves around modifying the compute_post_request and compute_get_request functions to send HTTP requests to the server. The core logic is as follows:

    Authentication and Authorization: Any command that interacts with the library requires the user to be logged in and possess a valid JWT token for access. If either condition is unmet, the command is not sent, and an appropriate error is returned.

    TCP Connections: Each command opens a new TCP connection to the server and closes it immediately after receiving the response, ensuring no long-term connections are maintained unnecessarily.

    Command Implementation: The commands follow a three-step process:
        Gathering arguments from stdin (if required),
        Sending the HTTP request and waiting for the server's response,
        Parsing the server's response and displaying either a success message or the relevant error.

The project utilizes the nlohmann JSON library due to its simple and intuitive functions for handling JSON message construction and parsing. This setup ensures clear and efficient communication between the client and the server, with appropriate error handling at each step.