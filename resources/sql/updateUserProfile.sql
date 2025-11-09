UPDATE
    users
SET
    first_name = :fn,
    middle_name = :mn,
    last_name = :ln,
    email = :email,
    full_name = CONCAT(:ln, ' ', :fn, ' ', :mn)
WHERE
    id = :id
