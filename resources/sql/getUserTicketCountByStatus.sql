SELECT
    s.name AS status,
    COUNT(*) AS count
FROM
    tickets t
JOIN statuses s ON
    t.status_id = s.id
WHERE
    t.assignee_id = :userId
GROUP BY
    s.name
ORDER BY
    s.name
