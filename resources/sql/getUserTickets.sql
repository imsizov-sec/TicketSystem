SELECT
    t.id,
    t.title,
    p.name AS project,
    pr.name AS priority,
    s.name AS status
FROM
    tickets t
LEFT JOIN projects p
    ON t.project_id = p.id
LEFT JOIN priorities pr
    ON t.priority_id = pr.id
LEFT JOIN statuses s
    ON t.status_id = s.id
WHERE
    (t.assignee_id = :userId
    OR t.watcher_id = :userId)
    AND s.id != 3
