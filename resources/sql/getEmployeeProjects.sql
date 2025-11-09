SELECT DISTINCT
    p.id,
    p.name
FROM
    tickets t
JOIN projects p ON
    t.project_id = p.id
WHERE
    t.assignee_id = :userId
