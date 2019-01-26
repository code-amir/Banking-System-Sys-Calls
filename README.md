The project aims to develop a banking system that is user-friendly and multifunctional.
The project have the following functionalities:
- All account transactional details and customer information are stored in files.
- Account holders have to pass through a login system to enter their accounts.
- The application possess password-protected administrative access,thus
preventing the whole management system from unauthorized access.
- Three types of login created: normal user, joint account user,administrator;
- Once you login as administrator, you can be able to add, delete, modify, search for a
specific account details.
- Once the customer connect to the server, you get the login and password prompt.
After successful login, you get the menu for example:
Do you want to: Deposit, Withdraw, Balance Enquiry, Password Change, View details, Exit
- once you login as a joint account holder, proper file locking is implemented. If you
want to view the account details then read lock is set else if you want to withdraw
or deposit then write lock is used to protect the critical data section.
- Use socket programming – Server maintains the data base and service multiple clients
concurrently. Client program can connect to the server and access their specific account
details.
- Used system calls instead of Library functions wherever it is possible in the project:
Process Management, File Management, File Locking, Multithreading and Inter Process
Communication Mechanisms.