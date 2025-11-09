SELECT
    id,
    name
FROM
    projects
WHERE
    department_id = (
        SELECT
            department_id
        FROM
            users
        WHERE
            id = :userId
    )
