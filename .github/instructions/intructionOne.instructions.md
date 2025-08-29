---
applyTo: '**'
---
Provide project context and coding guidelines that AI should follow when generating code, answering questions, or reviewing changes.

1. **Project Context**:
   - The project involves an ESP32S3-based motor controller that communicates with a master device (AGV_SAMI) via serial communication.
   - The motor controller supports both RPM-based and legacy PWM control methods.
   - PID control parameters (Kp, Ki, Kd) are used for motor speed regulation.


2. **Coding Guidelines**:
   - Use clear and descriptive variable names.
   - Include comments to explain the purpose of code blocks for important logic.
   - Follow consistent formatting and indentation for readability.
   - Validate and sanitize all incoming data from the master device.
   - Implement error handling
   - Optimize code for performance, especially in time-critical sections (e.g., motor control loops).
   - Keep the code modular and organized, separating different functionalities into distinct functions.
   - dont use delay() for timing; prefer non-blocking techniques (e.g., millis()).
   - read all code and understand its functionality before making changes.
   - understand context and requirements before implementing changes.
   - serial0 for agv_sami / master esp
   - serial1 for ESP32_Motor_Controller_Slave