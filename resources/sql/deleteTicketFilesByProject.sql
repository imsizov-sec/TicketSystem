DELETE
	f
FROM
	ticket_files f
JOIN tickets t ON
	f.ticket_id = t.id
WHERE
	t.project_id = :projectId
