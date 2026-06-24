# L4-load_balancer: balances traffic across various servers

# L4-load_balancer

This project implements an L4 (Transport Layer) load balancer designed to distribute incoming network traffic across multiple backend servers efficiently. It aims to enhance service availability, reliability, and scalability by preventing any single server from becoming a bottleneck. The system is built with a high-performance C++ backend and an interactive Python-based web dashboard for real-time monitoring.

## Key Features & Benefits

*   **Traffic Distribution**: Intelligently routes incoming requests to available backend servers, ensuring even load distribution.
*   **Backend Server Management**: Supports managing a dynamic pool of backend servers, potentially including health checks to ensure traffic is only sent to healthy instances.
*   **Performance Metrics**: Collects and exposes various metrics related to load balancer performance and backend server activity.
*   **Real-time Dashboard**: A comprehensive web-based dashboard provides live insights into traffic distribution, server load, and system health using `Chart.js` for visualization and WebSockets for real-time updates.
*   **High Concurrency**: Leverages a C++ thread pool for efficient handling of concurrent connections and requests, maximizing throughput.
*   **Modular Design**: Features a robust C++ core for load balancing logic and a separate Python FastAPI application for the monitoring dashboard, allowing for independent development and scaling.

## Technologies

### Languages

*   C++
*   Python

### Frameworks

*   **Python Framework**: FastAPI
*   **JavaScript Library**: Chart.js (for dashboard visualizations)

## Prerequisites & Dependencies

To build and run this project, you will need the following:

### General
*   `git` (for cloning the repository)

### C++ Backend
*   **C++ Compiler**: A C++17 compatible compiler (e.g., `g++` or `clang`).
*   **`make`**: A build automation tool (common for C++ projects, though not explicitly provided in the structure, it's a typical approach).
*   **Standard C++ Libraries**: Potentially POSIX threads (`pthread`) for concurrent operations.

### Python Dashboard
*   **Python 3.8+**: The Python interpreter.
*   **`pip`**: Python package installer (usually comes with Python).
*   **Python Dependencies**: The following packages are required and listed in `dashboard/requirements.txt`:
    *   `fastapi`: For building the web API and WebSocket server.
    *   `uvicorn`: An ASGI server to run FastAPI applications.
    *   `jinja2`: For templating the HTML dashboard.
    *   `websockets`: For real-time communication between the dashboard and the client.

## Installation & Setup

Follow these steps to get the L4 Load Balancer up and running on your local machine.

### 1. Clone the Repository

Start by cloning the project repository to your local machine:

```bash
git clone https://github.com/mohit-79/L4-load_balancer.git
cd L4-load_balancer
```

### 2. Build the C++ Load Balancer Backend

Navigate to the project root and compile the C++ source files.

*(Note: A `Makefile` is not explicitly provided in the given project structure. The following assumes a standard compilation process. You might need to adjust the command based on your specific build system or if a `Makefile` is added later.)*

```bash
# Example compilation command (adjust as necessary)
# This command assumes the main C++ source files are in the 'l4/' directory
# and headers are in 'include/'.
g++ -std=c++17 -pthread -Iinclude/ l4/*.cpp -o l4_balancer
```
After successful compilation, you should have an executable named `l4_balancer` (or similar, depending on your chosen output name) in your project root directory.

### 3. Set up and Run the Python Dashboard

The dashboard provides a graphical interface to monitor the load balancer's activity in real-time.

```bash
cd dashboard
pip install -r requirements.txt
uvicorn server:app --host 127.0.0.1 --port 8000 --reload
```
The `--reload` flag is optional and useful for development, as it automatically restarts the server on code changes. Once the server starts, you should see output indicating that the FastAPI application is running and accessible.

## Usage

### Running the Load Balancer

First, ensure you have your backend servers running that the load balancer will distribute traffic to. The exact configuration for backend servers (e.g., IP addresses, ports) would typically be provided to the C++ executable.

To start the load balancer:

```bash
# From the project root where 'l4_balancer' was compiled
./l4_balancer # Add any necessary arguments here for backend configuration
```
*   *(Note: The actual command-line arguments or configuration file format for defining and configuring backend servers are not specified in the provided details. You will need to refer to the C++ source code, particularly files like `backend_pool.h`, for specific configuration methods.)*

### Accessing the Dashboard

With the Python dashboard server running (as per the "Installation & Setup" section), open your web browser and navigate to:

[http://127.0.0.1:8000](http://127.0.0.1:8000)

This will load `dashboard/index.html`, displaying real-time metrics and visualizations of the load balancer's activity. The dashboard utilizes WebSockets to receive live updates from the `dashboard/server.py` backend, providing dynamic data without page refreshes.

You can also test the raw WebSocket connection directly via `dashboard/test.html` by opening it in your browser and ensuring the WebSocket URL inside the script matches your running dashboard server (`ws://127.0.0.1:8000/ws`).

## Configuration Options

### C++ Load Balancer Backend

The core C++ load balancer needs to be configured with details about the backend servers it will manage. Common methods for this include:
*   **Command-line arguments**: Passing server IPs and ports directly when starting the `l4_balancer` executable.
*   **Configuration file**: Specifying backend details in an external file (e.g., JSON, YAML, or a custom format) that the load balancer reads at startup.
*   **Environment variables**: Using environment variables to provide configuration parameters.

*(Please refer to the C++ source code, particularly `backend_pool.h` and the main `l4` executable's entry point, for the precise configuration methods implemented.)*

### Python Dashboard

*   **Host and Port**: The dashboard server's listening address and port can be configured using `uvicorn` command-line arguments when starting the server:
    ```bash
    uvicorn server:app --host <your_host> --port <your_port>
    ```
    (Default: `127.0.0.1:8000`)

## Contributing

Contributions are welcome! If you'd like to contribute to the L4-load_balancer project, please follow these guidelines:

1.  **Fork the repository**: Click the "Fork" button at the top right of this page to create your copy.
2.  **Create a new branch**: For each feature or bug fix, create a dedicated branch:
    ```bash
    git checkout -b feature/your-feature-name
    # or
    git checkout -b bugfix/issue-description
    ```
3.  **Implement your changes**: Make your desired modifications, ensuring code quality and adherence to existing coding styles.
4.  **Commit your changes**: Write clear and concise commit messages.
    ```bash
    git commit -m 'feat: Add new feature for X'
    ```
5.  **Push to the branch**: Upload your changes to your forked repository:
    ```bash
    git push origin feature/your-feature-name
    ```
6.  **Open a Pull Request**: Go to the original repository on GitHub and open a pull request from your new branch. Describe your changes thoroughly.

Please ensure your contributions are well-tested and documented where appropriate.

## License

This project is currently **unlicensed**. There is no explicit license granted for redistribution, modification, or commercial use. While you are free to fork and use this repository, it is recommended to contact the owner for any licensing inquiries or before deploying in production environments.

## Acknowledgments

*   [FastAPI](https://fastapi.tiangolo.com/): For providing an incredibly fast and modern Python web framework.
*   [Uvicorn](https://www.uvicorn.org/): For being a lightning-fast ASGI server that powers the dashboard.
*   [Jinja2](https://jinja.palletsprojects.com/): For flexible and powerful templating in the web dashboard.
*   [WebSockets (Python package)](https://websockets.readthedocs.io/): For enabling real-time bidirectional communication in the dashboard.
*   [Chart.js](https://www.chartjs.org/): For allowing us to create beautiful and interactive charts for data visualization in the web dashboard.
