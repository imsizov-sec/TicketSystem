SELECT
    id,
    first_name,
    ROLE
FROM
    ticket_system.users
WHERE
    email = :email
    AND password = SHA2(:password, 256)
