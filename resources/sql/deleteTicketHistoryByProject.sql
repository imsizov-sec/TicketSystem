DELETE
	h
FROM
	ticket_history h
JOIN tickets t ON
	h.ticket_id = t.id
WHERE
	t.project_id = :projectId
